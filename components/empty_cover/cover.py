import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import cover

empty_cover_ns = cg.esphome_ns.namespace("empty_cover")
EmptyCover = empty_cover_ns.class_("EmptyCover", cover.Cover, cg.Component)

CONFIG_SCHEMA = cover.cover_schema(EmptyCover).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await cover.new_cover(config)
    await cg.register_component(var, config)
