import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import valve, button, text_sensor
from esphome.const import CONF_ID, ENTITY_CATEGORY_DIAGNOSTIC, DEVICE_CLASS_WATER, CONF_NAME

AUTO_LOAD = ["valve", "button", "text_sensor"]
DEPENDENCIES = ["output", "valve"]
CODEOWNERS = ["@mhetzi"]
MULTI_CONF = True

CONF_VDMOT_HUB_ID = "vdmot_hub_id"
CONF_INA219_ID = "ina219_id"
CONF_INA219_CURRENT_ID = "ina219_current_id"
CONF_OUTPUT_CLOSE_ID = "output_close_id"
CONF_OUTPUT_OPEN_ID = "output_open_id"
CONF_MAX_CURRENT = "max_motor_current"

from esphome.components.ina219.sensor import (
    INA219Component
)

from esphome.components.output import (
    BinaryOutput,
)

from esphome.components.sensor import (
    Sensor,
)

vdmot_valve_ns = cg.esphome_ns.namespace("vdmot_valve")
VdmotHub = vdmot_valve_ns.class_("VdmotHub", cg.Component)
VdmotValve = vdmot_valve_ns.class_("VdmotValve", valve.Valve)
VdmotButtonCal = vdmot_valve_ns.class_("VdmotButtonCal", button.Button)

ButtonJob = vdmot_valve_ns.enum("OneJob")
OneJobs = {
    "JOB_CALIBRATE": ButtonJob.JOB_CALIBRATE,
    "JOB_MAINTENANCE": ButtonJob.JOB_MAINTENANCE,
    "JOB_NOTHING": ButtonJob.JOB_NOTHING
}

CONFIG_SCHEMA = valve.valve_schema(VdmotHub).extend(
    {
        cv.Required(CONF_INA219_ID): cv.use_id(INA219Component),
        cv.Required(CONF_INA219_CURRENT_ID): cv.use_id(Sensor),
        cv.Required(CONF_OUTPUT_CLOSE_ID): cv.use_id(BinaryOutput),
        cv.Required(CONF_OUTPUT_OPEN_ID): cv.use_id(BinaryOutput),
        cv.Optional(CONF_MAX_CURRENT, default=0.03): cv.float_range(min=0.001, max=2.0),
        cv.Optional("valve"): valve.valve_schema(
            VdmotValve,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            device_class=DEVICE_CLASS_WATER
        ),
        cv.Optional("calibrate"): button.button_schema(
            VdmotButtonCal,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional("maintenance"): button.button_schema(
            VdmotButtonCal,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional("status"): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        )
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    ina219 = await cg.get_variable(config[CONF_INA219_ID])
    ina_current = await cg.get_variable(config[CONF_INA219_CURRENT_ID])
    output_close = await cg.get_variable(config[CONF_OUTPUT_CLOSE_ID])
    output_open = await cg.get_variable(config[CONF_OUTPUT_OPEN_ID])
    max_motor_current = config.get(CONF_MAX_CURRENT)

    pipe = cg.new_Pvariable(config[CONF_ID])
    cg.add(pipe.set_ina219(ina219, ina_current))
    cg.add(pipe.set_output(output_close, output_open))
    cg.add(pipe.set_max_current(max_motor_current))
    await cg.register_component(pipe, config)

    if "valve" in config.keys():
        steam = await valve.new_valve(config.get("valve"))
        cg.add(pipe.set_valve(steam))
        cg.add(steam.set_parent(pipe))

    if "calibrate" in config.keys():
        btn = await button.new_button(config["calibrate"])
        cg.add(btn.set_parent(pipe, OneJobs["JOB_CALIBRATE"]))

    if "maintenance" in config.keys():
        btn = await button.new_button(config["maintenance"])
        cg.add(btn.set_parent(pipe, OneJobs["JOB_MAINTENANCE"]))
    
    if "status" in config.keys():
        txt = await text_sensor.new_text_sensor(config["status"])
        cg.add(pipe.set_txt_status(txt))
