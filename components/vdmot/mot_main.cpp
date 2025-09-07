#include "mot_main.h"

namespace esphome {
namespace vdmot_valve {

static const char *TAG = "vdmot_valve.hub";

void VdmotHub::set_output(output::BinaryOutput *output_close, output::BinaryOutput *output_open) {
    this->output_close_ = output_close;
    this->output_open_ = output_open;
    ESP_LOGD(TAG, "Outputs set");
}

void VdmotHub::set_ina219(ina219::INA219Component *ina219, sensor::Sensor *inaCurrentSens) {
    this->ina219_ = ina219;
    this->inaCurrentSensor_ = inaCurrentSens;
    ESP_LOGD(TAG, "INA219 set");
}

float VdmotHub::get_new_measured_current() {
    if (this->ina219_ != nullptr) {
        if (this->inaCurrentSensor_ != nullptr) {
            this->ina219_->update();
            return this->inaCurrentSensor_->get_raw_state();
        } else {
            ESP_LOGW(TAG, "INA219 current sensor not available");
            return NAN;
        }
    } else {
        ESP_LOGW(TAG, "INA219 component not set");
        return NAN;
    }
}

void VdmotHub::calibrate() {
    if (this->currentState != VDMOT_STATE_IDLE) {
        ESP_LOGW(TAG, "Cannot calibrate while not in IDLE state");
        return;
    }
    ESP_LOGI(TAG, "Starting calibration");
    this->currentState = VDMOT_STATE_RESET;
    this->max_moving_time_ = 0;
    this->move_start_time_ = 0;
    this->current_move_time_ = 0;
    this->last_position_ = 0.1f; // Unknown position
    this->sendValveState(valve::ValveOperation::VALVE_OPERATION_CLOSING);
}

void VdmotHub::loop() {
    if (this->currentState == VDMOT_STATE_IDLE) {
        return;
    }

    uint64_t now = millis();
    if (now - this->last_loop_ < 200 && this->currentState != VDMOT_STATE_IDLE) {
        return;
    }
    this->last_loop_ = now;
    auto current = this->get_new_measured_current();
    bool overcurrent = !std::isnan(current) && current > this->max_current_;

    switch (this->currentState) {
        case VDMOT_STATE_RESET:
            if (this->max_moving_time_ < now) {
                ESP_LOGI(TAG, "Waited, now starting zeroing");
                this->currentState = VDMOT_STATE_ZEROING;
                this->sendValveState(valve::ValveOperation::VALVE_OPERATION_OPENING);
            } else if (this->max_moving_time_ == 0 || this->move_start_time_ == 0) {
                this->setOutputs(false, false);
                ESP_LOGD(TAG, "Starting wait to zero");
                this->max_moving_time_ = now + 10000; // Wait max 10s to start zeroing
                this->move_start_time_ = now;
            } else {
                ESP_LOGD(TAG, "Waiting to start zeroing");
            }
            break;
        case VDMOT_STATE_ZEROING:
            ESP_LOGD(TAG, "State: ZEROING");
            this->setOutputs(false, true);
            if (overcurrent) {
                ESP_LOGD(TAG, "Zero position reached");
                this->setOutputs(false, false);
                this->currentState = VDMOT_STATE_MOVING_TO_CAL;
                this->last_position_ = 0.0f;
                this->max_moving_time_ = now;
                this->sendValveState(valve::ValveOperation::VALVE_OPERATION_IDLE);
            }
            break;
        case VDMOT_STATE_MOVING_TO_CAL:
            if (now - this->max_moving_time_ > 2000) {
                this->currentState = VDMOT_STATE_CALIBRATING;
                ESP_LOGD(TAG, "State: MOVING_TO_CAL");
                this->sendValveState(valve::ValveOperation::VALVE_OPERATION_CLOSING);
            }
            break;
        case VDMOT_STATE_CALIBRATING:
            ESP_LOGD(TAG, "State: CALIBRATING");
            this->setOutputs(true, false);
            if (overcurrent) {
                ESP_LOGD(TAG, "Max position reached");
                this->setOutputs(false, false);
                this->currentState = VDMOT_STATE_IDLE;
                this->last_position_ = 0.0f;
                this->current_position_ = 0.0f;
                this->max_moving_time_ = now - this->max_moving_time_;
                ESP_LOGD(TAG, "Max moving time: %llu ms", this->max_moving_time_);
                this->sendValveState(valve::ValveOperation::VALVE_OPERATION_IDLE);
                this->set_position(0.5f); // Move to middle position after calibration
            }
            break;
        case VDMOT_STATE_MOVING:{
                ESP_LOGD(TAG, "State: MOVING");
                bool move_time_reached = (now - this->move_start_time_ > this->current_move_time_) || (now - this->move_start_time_ > this->max_moving_time_);
                if (overcurrent || move_time_reached) {
                    this->setOutputs(false, false);
                    this->currentState = VDMOT_STATE_IDLE;
                    this->sendValveState(valve::ValveOperation::VALVE_OPERATION_IDLE);
                    this->current_move_time_ = now - this->move_start_time_;
                    ESP_LOGD(TAG, "Target position reached. Remaining time: %d ms", this->current_move_time_);
                    this->last_position_ = this->current_position_;
                }
            }
            break;
        case VDMOT_STATE_ERROR:
            ESP_LOGD(TAG, "State: ERROR");
            this->setOutputs(false, false);
            break;
        default:
            ESP_LOGE(TAG, "Unknown state");
            this->currentState = VDMOT_STATE_ERROR;
            break;
    }

}

void VdmotHub::sendValveState(valve::ValveOperation operation) {
    if (this->valve_ != nullptr) {
        this->valve_->position = this->last_position_;
        this->valve_->current_operation = operation;
        this->valve_->publish_state(false);
    } else {
        ESP_LOGE(TAG, "Valve not set");
    }
}

void VdmotHub::setOutputs(bool doClose, bool doOpen) {
    if (this->output_close_ != nullptr && this->output_open_ != nullptr) {
        doOpen = doClose ? false : doOpen;
        this->output_close_->set_state(doClose);
        this->output_open_->set_state(doOpen);
    } else {
        ESP_LOGE(TAG, "Outputs not set");
    }
}

void VdmotHub::set_position(float position) {
    /*
     position 0.0 = closed
     position 1.0 = open
     position -100 = stop
    */
   if (position == -100) {
       this->setOutputs(false, false);
       this->currentState = VDMOT_STATE_IDLE;
       return;
   }

    if (position < 0.0f || position > 1.0f) {
         ESP_LOGE(TAG, "Invalid position value: %.2f", position);
         return;
    }

    if (this->currentState != VDMOT_STATE_IDLE) {
        ESP_LOGW(TAG, "Cannot move valve while not in IDLE state");
        return;
    }

    if (position == this->last_position_) {
        ESP_LOGD(TAG, "Position already at %.2f", position);
        return;
    }

    auto diff = position - this->last_position_;

    this->current_move_time_ = abs(diff) * this->max_moving_time_;
    if (this->current_move_time_ < MINIMAL_MOVEMENT_MS) {
        ESP_LOGW(TAG, "Movement time too short: %d ms", this->current_move_time_);
        return;
    }

    this->current_position_ = position;

    ESP_LOGD(TAG, "Moving from %.2f to %.2f, time: %d ms", this->last_position_, this->current_position_, this->current_move_time_);
    this->move_start_time_ = millis();
    this->currentState = VDMOT_STATE_MOVING;
    if (diff > 0) {
        this->setOutputs(false, true);
        this->sendValveState(valve::ValveOperation::VALVE_OPERATION_OPENING);
    } else {
        this->setOutputs(true, false);
        this->sendValveState(valve::ValveOperation::VALVE_OPERATION_CLOSING);
    }

}

void VdmotHub::dump_config() {
    ESP_LOGCONFIG(TAG, "Vdmot Hub:");
    if (this->ina219_ != nullptr) {
        ESP_LOGCONFIG(TAG, "  INA219: %#010x", this->ina219_);
        ESP_LOGCONFIG(TAG, "  INA219 Current Sensor: %#010x", this->inaCurrentSensor_);
    } else {
        ESP_LOGW(TAG, "  INA219 not set");
    }
    if (this->output_close_ != nullptr && this->output_open_ != nullptr) {
        ESP_LOGCONFIG(TAG, "  Outputs:");
        ESP_LOGCONFIG(TAG, "    Close: %#010x", this->output_close_);
        ESP_LOGCONFIG(TAG, "    Open: %#010x", this->output_open_);
    } else {
        ESP_LOGW(TAG, "  Outputs not set");
    }
    if (this->valve_ != nullptr) {
        ESP_LOGCONFIG(TAG, "  Valve: %#010x", this->valve_);
    } else {
        ESP_LOGW(TAG, "  Valve not set");
    }
    ESP_LOGCONFIG(TAG, "  Max current: %.2f A", this->max_current_);
    ESP_LOGCONFIG(TAG, "  Current state: %s", getVdmotStateString(this->currentState));
    ESP_LOGCONFIG(TAG, "  Last position: %.2f", this->last_position_);
    ESP_LOGCONFIG(TAG, "  Current position: %.2f", this->current_position_);
    ESP_LOGCONFIG(TAG, "  Max moving time: %llu ms", this->max_moving_time_);
    ESP_LOGCONFIG(TAG, "  Current move time: %d ms", this->current_move_time_);
}

} //namespace vdmot_valve
} //namespace esphome