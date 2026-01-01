import esphome.codegen as cg
import esphome.config_validation as cv

CODEOWNERS = ["@LXXero"]
AUTO_LOAD = ["light"]

sequencer_ns = cg.esphome_ns.namespace("sequencer")

# Force-register the effects by importing light.py
from .light import (
    fire_code_effect_to_code,
    step_seq_effect_to_code,
    march_effect_to_code,
)

CONFIG_SCHEMA = cv.Schema({})

async def to_code(config):
    pass
