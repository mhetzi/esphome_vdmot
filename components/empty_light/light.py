import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, output
from esphome.components.light import LightType
from esphome.const import CONF_OUTPUT_ID, CONF_OUTPUT

empty_light_ns = cg.esphome_ns.namespace("empty_light")
EmptyLightOutput = empty_light_ns.class_("EmptyLightOutput", light.LightOutput)

CONFIG_SCHEMA = light.light_schema(
    EmptyLightOutput, type_=LightType.BRIGHTNESS_ONLY
).extend(
    {
        cv.Required(CONF_OUTPUT): cv.use_id(output.FloatOutput),
    }
)


async def to_code(config):
    var = await light.new_light(config)
    out = await cg.get_variable(config[CONF_OUTPUT])
    cg.add(var.set_output(out))
