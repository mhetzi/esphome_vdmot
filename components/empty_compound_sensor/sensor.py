import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, UNIT_EMPTY, ICON_EMPTY

empty_compound_sensor_ns = cg.esphome_ns.namespace("empty_compound_sensor")
EmptyCompoundSensor = empty_compound_sensor_ns.class_(
    "EmptyCompoundSensor", cg.PollingComponent
)

CONF_SENSOR1 = "sensor1"
CONF_SENSOR2 = "sensor2"
CONF_SENSOR3 = "sensor3"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(EmptyCompoundSensor),
        cv.Optional(CONF_SENSOR1): sensor.sensor_schema(
            EmptyCompoundSensor,
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_EMPTY,
            accuracy_decimals=1,
        ).extend(),
        cv.Optional(CONF_SENSOR2): sensor.sensor_schema(
            EmptyCompoundSensor,
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_EMPTY,
            accuracy_decimals=1,
        ).extend(),
        cv.Optional(CONF_SENSOR3): sensor.sensor_schema(
            EmptyCompoundSensor,
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_EMPTY,
            accuracy_decimals=1,
        ).extend(),
    }
).extend(cv.polling_component_schema("60s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_SENSOR1 in config:
        sens = await sensor.new_sensor(config[CONF_SENSOR1])
        cg.add(var.set_sensor1(sens))

    if CONF_SENSOR2 in config:
        sens = await sensor.new_sensor(config[CONF_SENSOR2])
        cg.add(var.set_sensor2(sens))

    if CONF_SENSOR3 in config:
        sens = await sensor.new_sensor(config[CONF_SENSOR3])
        cg.add(var.set_sensor3(sens))
