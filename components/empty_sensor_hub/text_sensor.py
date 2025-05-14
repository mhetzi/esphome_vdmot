import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from . import EmptySensorHub, CONF_EMPTY_SENSOR_HUB_ID

DEPENDENCIES = ["empty_sensor_hub"]

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema()
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
    await text_sensor.register_text_sensor(var, config)
    cg.add(paren.register_text_sensor(var))
