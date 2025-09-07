#include "button_cal.h"

namespace esphome {
namespace vdmot_valve {

static const char *TAG = "vdmot.button_cal";

void VdmotButtonCal::set_parent(VdmotHub *parent) {
    this->parent_ = parent;
}

void VdmotButtonCal::press_action() {
    if (this->parent_ == nullptr) {
        ESP_LOGE(TAG, "Parent hub not set");
        return;
    }
    ESP_LOGI(TAG, "Calibration started");
    this->parent_->calibrate();
}

} //namespace vdmot_valve
} //namespace esphome