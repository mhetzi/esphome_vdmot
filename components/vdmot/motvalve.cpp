#include "esphome/core/log.h"
#include "motvalve.h"

namespace esphome {
namespace vdmot_valve {

static const char *TAG = "vdmot.valve";

valve::ValveTraits VdmotValve::get_traits() {
    valve::ValveTraits traits;
    traits.set_supports_position(true);
    traits.set_supports_stop(true);
    traits.set_supports_toggle(false);
    traits.set_is_assumed_state(false);
    return traits;
}

void VdmotValve::dump_config(){
    ESP_LOGCONFIG(TAG, "Vdmot Valve");
}

void VdmotValve::set_parent(vdmot_valve::VdmotHub *parent) {
    this->parent_ = parent;
}

void VdmotValve::setup() {
    this->position = valve::VALVE_CLOSED;
    this->current_operation = valve::VALVE_OPERATION_IDLE;
}

void VdmotValve::control(const valve::ValveCall &call) {
    ESP_LOGD(TAG, "Control called");
    if (this->parent_ == nullptr) {
        ESP_LOGE(TAG, "Parent hub not set");
        return;
    }
    if (call.get_stop()) {
        ESP_LOGD(TAG, "Stopping valve");
        this->current_operation = valve::VALVE_OPERATION_IDLE;
        this->parent_->set_position(-100);
        return;
    }
    if (call.get_position().has_value()) {
        float position = call.get_position().value();
        if (position < valve::VALVE_CLOSED || position > valve::VALVE_OPEN) {
            ESP_LOGE(TAG, "Invalid position value: %.2f", position);
            return;
        }
        ESP_LOGD(TAG, "Setting position to %.2f", position);
        if (position > this->position) {
            this->current_operation = valve::VALVE_OPERATION_OPENING;
        } else if (position < this->position) {
            this->current_operation = valve::VALVE_OPERATION_CLOSING;
        } else {
            this->current_operation = valve::VALVE_OPERATION_IDLE;
        }
        this->parent_->set_position(position);
        this->position = position;
        this->publish_state(false);
        return;
    }
}

void VdmotValve::set_position(float position, valve::ValveOperation operation) {
    this->position = position;
    this->current_operation = operation;
    this->publish_state(false);
}

} //namespace empty_switch
} //namespace esphome