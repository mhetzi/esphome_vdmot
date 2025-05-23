#include "empty_automation.h"
#include "esphome/core/log.h"

namespace esphome {
namespace empty_automation {

static const char *const TAG = "empty_automation";

void EmptyAutomation::add_on_state_callback(std::function<void(bool)> &&callback) {
  this->state_callback_.add(std::move(callback));
}

void EmptyAutomation::set_state(bool state) {
  ESP_LOGD(TAG, "Set state to %s", TRUEFALSE(state));
  this->state = state;
  this->state_callback_.call(state);
}

}  // namespace empty_automation
}  // namespace esphome
