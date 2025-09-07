#pragma once
#include "esphome/core/component.h"
#include "esphome/components/button/button.h"
#include "./mot_main.h"

namespace esphome {
namespace vdmot_valve {

class VdmotHub; // Forward declaration
class VdmotButtonCal : public button::Button, public Component {
 public:    
    void set_parent(VdmotHub *parent);

 private:
    VdmotHub *parent_{nullptr};

    void press_action() override;
};

} //namespace vdmot_valve
} //namespace esphome