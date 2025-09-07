#pragma once

#include "esphome/core/component.h"
#include "esphome/components/valve/valve.h"
#include "./mot_main.h"

namespace esphome {
namespace vdmot_valve {

class VdmotHub; // Forward declaration
class VdmotValve : public valve::Valve, public Component {

 public:
  valve::ValveTraits get_traits() override;
  void setup() override;
  void dump_config() override;

  void set_parent(VdmotHub *parent);

friend class VdmotHub;
protected:
  void control(const valve::ValveCall &call) override;
  void set_position(float position, valve::ValveOperation operation);

private:
  VdmotHub *parent_{nullptr};
};

} //namespace empty_switch
} //namespace esphome