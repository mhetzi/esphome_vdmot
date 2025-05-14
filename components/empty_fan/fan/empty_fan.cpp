#include "empty_fan.h"
#include "esphome/core/log.h"

namespace esphome {
namespace empty_fan {

static const char *TAG = "empty_fan.fan";

void EmptyFan::setup() {
  // Construct traits
  this->traits_ = fan::FanTraits(this->direction_output_ != nullptr, false, this->oscillating_output_ != nullptr, 0);
}

void EmptyFan::dump_config() { ESP_LOGCONFIG(TAG, "Empty fan"); }

void EmptyFan::control(const fan::FanCall &call) {
  if (call.get_state().has_value()) {
    this->state = *call.get_state();
  }
  if (call.get_oscillating().has_value()) {
    this->oscillating = *call.get_oscillating();
  }
  if (call.get_direction().has_value()) {
    this->direction = *call.get_direction();
  }

  this->write_state_();
  this->publish_state();
}

void EmptyFan::write_state_() {
  if (this->oscillating_output_ != nullptr) {
    this->oscillating_output_->set_state(this->oscillating);
  }
  if (this->direction_output_ != nullptr) {
    this->direction_output_->set_state(this->direction == fan::FanDirection::REVERSE);
  }
}

}  // namespace empty_fan
}  // namespace esphome
