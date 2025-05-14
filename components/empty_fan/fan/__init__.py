import esphome.codegen as cg
from esphome.components import fan, output
import esphome.config_validation as cv
from esphome.const import (
    CONF_DIRECTION_OUTPUT,
    CONF_OSCILLATION_OUTPUT,
    CONF_OUTPUT,
    CONF_OUTPUT_ID,
)
from .. import empty_fan_ns

EmptyFan = empty_fan_ns.class_("EmptyFan", cg.Component)

CONFIG_SCHEMA = (
    fan.fan_schema(EmptyFan)
    .extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(EmptyFan),
            cv.Required(CONF_OUTPUT): cv.use_id(output.BinaryOutput),
            cv.Optional(CONF_OSCILLATION_OUTPUT): cv.use_id(output.BinaryOutput),
            cv.Optional(CONF_DIRECTION_OUTPUT): cv.use_id(output.BinaryOutput),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await fan.new_fan(config)
    await cg.register_component(var, config)

    output_ = await cg.get_variable(config[CONF_OUTPUT])
    cg.add(var.set_output(output_))

    if CONF_OSCILLATION_OUTPUT in config:
        oscillation_output = await cg.get_variable(config[CONF_OSCILLATION_OUTPUT])
        cg.add(var.set_oscillating(oscillation_output))

    if CONF_DIRECTION_OUTPUT in config:
        direction_output = await cg.get_variable(config[CONF_DIRECTION_OUTPUT])
        cg.add(var.set_direction(direction_output))
