#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/components/ina219/ina219.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/valve/valve.h"

#include "motvalve.h"

namespace esphome {
namespace vdmot_valve {

enum VdmotState : uint8_t {
    VDMOT_STATE_RESET = 0,
    VDMOT_STATE_ZEROING,
    VDMOT_STATE_MOVING_TO_CAL,
    VDMOT_STATE_CALIBRATING,
    VDMOT_STATE_MOVING,
    VDMOT_STATE_IDLE,
    VDMOT_STATE_ERROR
};

static const char *getVdmotStateString(VdmotState state) {
    switch (state) {
        case VDMOT_STATE_RESET: return "RESET";
        case VDMOT_STATE_ZEROING: return "ZEROING";
        case VDMOT_STATE_MOVING_TO_CAL: return "Sleeping before CALIBRATING";
        case VDMOT_STATE_CALIBRATING: return "CALIBRATING";
        case VDMOT_STATE_MOVING: return "MOVING";
        case VDMOT_STATE_IDLE: return "IDLE";
        case VDMOT_STATE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

#define MINIMAL_MOVEMENT_MS 200

class VdmotValve; // Forward declaration
class VdmotHub : public Component {
    public:
        void set_ina219(ina219::INA219Component *ina219, sensor::Sensor *inaCurrentSens);
        void set_output(output::BinaryOutput *output_close, output::BinaryOutput *output_open);
        void set_max_current(float max_current) { this->max_current_ = max_current; }
        void set_valve(VdmotValve *valve) { this->valve_ = valve; }
        void loop() override;
        
        void dump_config() override;

        void calibrate();
        
    friend class VdmotValve;
    protected:
        float get_new_measured_current();
        void set_position(float position);
    private:
        float max_current_{1.0f}; // Max current in A

        output::BinaryOutput *output_close_{nullptr};
        output::BinaryOutput *output_open_{nullptr};

        ina219::INA219Component *ina219_{nullptr};
        sensor::Sensor *inaCurrentSensor_{nullptr};
        vdmot_valve::VdmotValve *valve_{nullptr};

        uint64_t last_loop_{0};
        VdmotState currentState{VDMOT_STATE_RESET};
        float last_position_{0.1f};
        float current_position_{0.0f};

        uint64_t max_moving_time_{0}; // Max time to move in ms
        int32_t current_move_time_{0}; // Current time to move in ms
        uint64_t move_start_time_{0};
        
        /* bool close or bool open */
        void setOutputs(bool close, bool open);
        void sendValveState(valve::ValveOperation operation);
};  

} //namespace vdmot_valve
} //namespace esphome 