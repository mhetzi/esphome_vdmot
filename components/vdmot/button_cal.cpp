#include "button_cal.h"

namespace esphome {
namespace vdmot_valve {

static const char *TAG = "vdmot.button_cal";

void VdmotButtonCal::set_parent(VdmotHub *parent, OneJob job) {
    this->parent_ = parent;
    this->myJob = job;
}

void VdmotButtonCal::press_action() {
    if (this->parent_ == nullptr) {
        ESP_LOGE(TAG, "Parent hub not set");
        return;
    }
    switch (this->myJob){
        case OneJob::JOB_CALIBRATE:
            ESP_LOGI(TAG, "Calibration started");
            this->parent_->calibrate();
            break;
        case OneJob::JOB_MAINTENANCE:
            ESP_LOGI(TAG, "Maintenance started");
            this->parent_->start_maintenance();
            break;
        default:
            ESP_LOGE(TAG, "You had one Job and still forgot!");
            break;
    }
}

} //namespace vdmot_valve
} //namespace esphome