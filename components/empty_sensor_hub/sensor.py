import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, UNIT_EMPTY, ICON_EMPTY
from . import EmptySensorHub, CONF_EMPTY_SENSOR_HUB_ID

DEPENDENCIES = ["empty_sensor_hub"]

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY, accuracy_decimals=1
    )
    .extend(
        {
            cv.GenerateID(CONF_EMPTY_SENSOR_HUB_ID): cv.use_id(EmptySensorHub),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    paren = await cg.get_variable(config[CONF_EMPTY_SENSOR_HUB_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    cg.add(paren.register_sensor(var))
