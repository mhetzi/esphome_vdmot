#pragma once
#include "esphome/core/component.h"
#include "esphome/components/button/button.h"
#include "./mot_main.h"

namespace esphome {
namespace vdmot_valve {

enum OneJob :uint8_t {
   JOB_CALIBRATE = 0,
   JOB_MAINTENANCE,
   JOB_NOTHING
};

class VdmotHub; // Forward declaration
class VdmotButtonCal : public button::Button, public Component {
 public:    
    void set_parent(VdmotHub *parent, OneJob job);

 private:
    VdmotHub *parent_{nullptr};
    OneJob myJob{JOB_NOTHING};

    void press_action() override;
};

} //namespace vdmot_valve
} //namespace esphome