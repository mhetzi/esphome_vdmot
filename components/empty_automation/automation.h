#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "empty_automation.h"

namespace esphome {
namespace empty_automation {

template<typename... Ts> class EmptyAutomationSetStateAction : public Action<Ts...> {
 public:
  explicit EmptyAutomationSetStateAction(EmptyAutomation *ea) : ea_(ea) {}
  TEMPLATABLE_VALUE(bool, state)

  void play(Ts... x) override {
    auto val = this->state_.value(x...);
    this->ea_->set_state(val);
  }

 protected:
  EmptyAutomation *ea_;
};

template<typename... Ts> class EmptyAutomationCondition : public Condition<Ts...> {
 public:
  EmptyAutomationCondition(EmptyAutomation *parent, bool state) : parent_(parent), state_(state) {}
  bool check(Ts... x) override { return this->parent_->state == this->state_; }

 protected:
  EmptyAutomation *parent_;
  bool state_;
};

class StateTrigger : public Trigger<bool> {
 public:
  explicit StateTrigger(EmptyAutomation *parent) {
    parent->add_on_state_callback([this](bool state) { this->trigger(state); });
  }
};

}  // namespace empty_automation
}  // namespace esphome
