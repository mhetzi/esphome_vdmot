from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PIN
from esphome.cpp_helpers import gpio_pin_expression

empty_gpio_component_ns = cg.esphome_ns.namespace("empty_gpio_component")
EmptyGPIOComponent = empty_gpio_component_ns.class_("EmptyGPIOComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(EmptyGPIOComponent),
        cv.Required(CONF_PIN): pins.gpio_output_pin_schema,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    pin = await gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_output_pin(pin))
