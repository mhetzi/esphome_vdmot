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
    if (this->flags_.state != VDMOT_STATE_IDLE && this->flags_.state != VDMOT_STATE_ERR_NO_CURRENT) {
        ESP_LOGW(TAG, "Cannot calibrate while not in IDLE state, current state: %s", getVdmotStateString(this->flags_.state));
        return;
    }
    ESP_LOGI(TAG, "Starting calibration");
    this->flags_.state = VDMOT_STATE_RESET;
    this->max_moving_time_ = 0;
    this->move_start_time_ = 0;
    this->current_move_time_ = 0;
    this->last_position_ = 0.1f; // Unknown position
    this->sendValveState(valve::ValveOperation::VALVE_OPERATION_CLOSING);
}

void VdmotHub::loop() {
    if (this->flags_.lastProcessedState != this->flags_.state){
        this->flags_.lastProcessedState = this->flags_.state;
        if (this->txtState != nullptr){
            this->txtState->publish_state(getVdmotStateAsString(this->flags_.state, this->flags_.maintenance));
        }
    }

    if (this->flags_.state == VDMOT_STATE_IDLE) {
        return;
    }

    uint64_t now = millis();
    if (now - this->last_loop_ < 100 && this->flags_.state != VDMOT_STATE_IDLE) {
        return;
    }
    this->last_loop_ = now;
    const auto current = this->get_new_measured_current();
    bool overcurrent = !std::isnan(current) && current > this->max_current_ && (this->flags_.output_close || this->flags_.output_open);
    const bool noCurrent = !std::isnan(current) && current < 0.002 && (this->flags_.output_close || this->flags_.output_open);

    this->flags_.noCurrentFlow = noCurrent ? this->flags_.noCurrentFlow + 1 : 0;
    if (this->flags_.noCurrentFlow > 10 ) {
        this->setOutputs(false, false);
        this->flags_.state = VDMOT_STATE_ERR_NO_CURRENT;
        return;
    }
    this->flags_.overcurrentCount = overcurrent ? this->flags_.overcurrentCount + 1 : 0;
    if (overcurrent){
        ESP_LOGD(TAG, "Overcurrent detected: %.4f > %.4f for %d steps.", current, this->max_current_, this->flags_.overcurrentCount);
    }
    overcurrent = this->flags_.overcurrentCount > 3;
    this->flags_.overcurrentCount = overcurrent ? 0 : this->flags_.overcurrentCount;

    switch (this->flags_.state) {
        case VDMOT_STATE_RESET:
            if (this->max_moving_time_ == 0 || this->move_start_time_ == 0) {
                this->setOutputs(false, false);
                ESP_LOGI(TAG, "Starting wait to zero");
                this->max_moving_time_ = now + 20000; // Wait max 10s to start zeroing
                this->move_start_time_ = now;
                this->flags_.noCurrentFlow = 0;
            } else if (this->max_moving_time_ < now) {
                ESP_LOGI(TAG, "Waited, now starting zeroing");
                this->flags_.state = VDMOT_STATE_ZEROING;
                this->sendValveState(valve::ValveOperation::VALVE_OPERATION_OPENING);
                this->max_moving_time_ = 0;
                this->move_start_time_ = 0;
            } else {
                ESP_LOGV(TAG, "Waiting to start zeroing");
            }
            break;
        case VDMOT_STATE_ZEROING:
            ESP_LOGV(TAG, "State: ZEROING");
            this->setOutputs(false, true);
            if (overcurrent) {
                ESP_LOGD(TAG, "Zero position reached");
                this->setOutputs(false, false);
                this->flags_.state = VDMOT_STATE_MOVING_TO_CAL;
                this->last_position_ = 0.0f;
                this->max_moving_time_ = now;
                this->sendValveState(valve::ValveOperation::VALVE_OPERATION_IDLE);
            }
            break;
        case VDMOT_STATE_MOVING_TO_CAL:
            if (now - this->max_moving_time_ > 2000) {
                this->flags_.state = VDMOT_STATE_CALIBRATING;
                ESP_LOGI(TAG, "State: MOVING_TO_CAL");
                this->setOutputs(false, false);
                this->max_moving_time_ = now;
                this->move_start_time_ = 0;
                this->sendValveState(valve::ValveOperation::VALVE_OPERATION_CLOSING);
            }
            break;
        case VDMOT_STATE_CALIBRATING:
            ESP_LOGV(TAG, "State: CALIBRATING");
            this->setOutputs(true, false);
            if (overcurrent) {
                ESP_LOGD(TAG, "Max position reached");
                this->setOutputs(false, false);
                this->flags_.state = VDMOT_STATE_IDLE;
                this->last_position_ = 0.0f;
                this->current_position_ = 0.0f;
                this->max_moving_time_ = now - this->max_moving_time_;
                ESP_LOGI(TAG, "Max moving time: %llu ms", this->max_moving_time_);
                this->sendValveState(valve::ValveOperation::VALVE_OPERATION_IDLE);
                this->set_position(0.5f); // Move to middle position after calibration
            }
            break;
        case VDMOT_STATE_MOVING:{
                ESP_LOGV(TAG, "State: MOVING");
                bool move_time_reached = (now - this->move_start_time_ > this->current_move_time_) || (now - this->move_start_time_ > this->max_moving_time_);
                if (overcurrent || move_time_reached) {
                    ESP_LOGD(TAG, "Move done. OC: %d, timed %d", overcurrent, move_time_reached);
                    this->setOutputs(false, false);
                    this->flags_.state = VDMOT_STATE_IDLE;
                    this->sendValveState(valve::ValveOperation::VALVE_OPERATION_IDLE);
                    this->current_move_time_ = (now - this->move_start_time_) - this->current_move_time_;
                    ESP_LOGI(TAG, "Target position reached. Remaining time: %d ms", this->current_move_time_);
                    this->last_position_ = this->current_position_;
                    switch (this->flags_.maintenance) {
                        case VDMOT_MAINTENANCE_BEGIN:
                            ESP_LOGI(TAG, "Maintenance: BEGIN -> MIDDLE");
                            this->flags_.maintenance = VDMOT_MAINTENANCE_MIDDLE;
                            this->flags_.lastProcessedState = VDMOT_STATE_IDLE;
                            this->set_position(0.0f, true); // Move to closed position
                            break;
                        case VDMOT_MAINTENANCE_MIDDLE:
                            ESP_LOGI(TAG, "Maintenance: MIDDLE -> END");
                            this->flags_.maintenance = VDMOT_MAINTENANCE_END;
                            this->flags_.lastProcessedState = VDMOT_STATE_IDLE;
                            this->set_position(1.0f, true); // Move to open position
                            break;
                        case VDMOT_MAINTENANCE_END:
                            ESP_LOGI(TAG, "Maintenance: END -> NONE");
                            this->flags_.maintenance = VDMOT_MAINTENANCE_NONE;
                            this->flags_.lastProcessedState = VDMOT_STATE_IDLE;
                            break;
                        case VDMOT_MAINTENANCE_NONE:
                        default:
                            // Nothing to do
                            break;
                    }
                }
            }
            break;
        case VDMOT_STATE_ERROR:
            ESP_LOGW(TAG, "State: ERROR");
            this->setOutputs(false, false);
            break;
        default:
            ESP_LOGE(TAG, "Unknown state");
            //this->flags_.state = VDMOT_STATE_ERROR;
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
        this->flags_.output_close = doClose;
        this->flags_.output_open = doOpen;
    } else {
        ESP_LOGE(TAG, "Outputs not set");
    }
}

void VdmotHub::start_maintenance() {
    if (this->flags_.state != VDMOT_STATE_IDLE) {
        ESP_LOGW(TAG, "Cannot start maintenance while not in IDLE state, current state: %s", getVdmotStateString(this->flags_.state));
        return;
    }
    ESP_LOGI(TAG, "Starting maintenance cycle");
    this->flags_.maintenance = VDMOT_MAINTENANCE_BEGIN;
    this->set_position(1.0f); // Move to open position
}

void VdmotHub::set_position(float position) {
    this->set_position(position, false);
}

void VdmotHub::set_position(float position, bool force) {
    /*
     position 0.0 = closed
     position 1.0 = open
     position -100 = stop
    */
   if (position == -100) {
       this->setOutputs(false, false);
       this->flags_.state = VDMOT_STATE_IDLE;
       return;
   }

    if (position < 0.0f || position > 1.0f || std::isnan(position)) {
         ESP_LOGE(TAG, "Invalid position value: %.2f", position);
         return;
    }

    if (this->flags_.state != VDMOT_STATE_IDLE) {
        ESP_LOGW(TAG, "Cannot move valve while not in IDLE state. State: %s, Force: %d", getVdmotStateString(this->flags_.state), force);
        if (!force)
            return;
    }

    if (position == this->last_position_) {
        ESP_LOGV(TAG, "Position already at %.2f", position);
        return;
    }

    auto diff = position - this->last_position_;

    this->current_move_time_ = abs(diff) * this->max_moving_time_;
    if (this->current_move_time_ < MINIMAL_MOVEMENT_MS) {
        ESP_LOGW(TAG, "Movement time too short: %d ms", this->current_move_time_);
        return;
    } else if (this->current_move_time_ > MAX_MOVE_MS){
        ESP_LOGW(TAG, "Movement time way too long %d ms", this->current_move_time_);
        return;
    }

    this->current_position_ = position;

    ESP_LOGD(TAG, "Moving from %.2f to %.2f, time: %d ms, force?: %d", this->last_position_, this->current_position_, this->current_move_time_, force);
    this->move_start_time_ = millis();
    this->flags_.state = VDMOT_STATE_MOVING;
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
    ESP_LOGCONFIG(TAG, "  INA219: %s", YESNO(this->ina219_));
    ESP_LOGCONFIG(TAG, "  INA219 Current Sensor: %s", YESNO(this->inaCurrentSensor_));
    ESP_LOGCONFIG(TAG, "  Outputs:");
    ESP_LOGCONFIG(TAG, "    Close: %s", YESNO(this->output_close_));
    ESP_LOGCONFIG(TAG, "    Open: %s" , YESNO(this->output_open_ ));
    ESP_LOGCONFIG(TAG, "  Valve: %s", YESNO(this->valve_));
    ESP_LOGCONFIG(TAG, "  Max current: %.2f A", this->max_current_);
    ESP_LOGCONFIG(TAG, "  Current state: %s", getVdmotStateString(this->flags_.state));
    ESP_LOGCONFIG(TAG, "  Last position: %.2f%%", this->last_position_);
    ESP_LOGCONFIG(TAG, "  Current position: %.2f%%", this->current_position_);
    ESP_LOGCONFIG(TAG, "  Max moving time: %llu ms", this->max_moving_time_);
    ESP_LOGCONFIG(TAG, "  Current move time: %d ms", this->current_move_time_);
}

} //namespace vdmot_valve
} //namespace esphome