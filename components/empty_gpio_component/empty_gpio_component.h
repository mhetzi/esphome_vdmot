#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace empty_gpio_component {

class EmptyGPIOComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_output_pin(GPIOPin *pin) { this->pin_ = pin; }

 protected:
  GPIOPin *pin_;
};

}  // namespace empty_gpio_component
}  // namespace esphome