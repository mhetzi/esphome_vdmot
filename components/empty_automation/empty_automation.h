#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace empty_automation {

class EmptyAutomation : public Component {
 public:
  void add_on_state_callback(std::function<void(bool)> &&callback);
  void set_state(bool state);

  bool state{false};

 protected:
  CallbackManager<void(bool)> state_callback_{};
};

}  // namespace empty_automation
}  // namespace esphome
