import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.light.effects import register_binary_effect
from esphome.components.output import BinaryOutput
from esphome.components.switch import Switch
from esphome.const import CONF_NAME, CONF_OUTPUT
from . import sequencer_ns

CONF_PATTERN = "pattern"
CONF_PULSE_MS = "pulse_ms"
CONF_GAP_MS = "gap_ms"
CONF_GROUP_GAP_MS = "group_gap_ms"
CONF_CYCLE_GAP_MS = "cycle_gap_ms"
CONF_STEP_MS = "step_ms"
CONF_SHORT_MS = "short_ms"
CONF_LONG_MS = "long_ms"
CONF_ON_MS = "on_ms"
CONF_OFF_MS = "off_ms"

# Config keys for chime_seq
CONF_OUTPUT_E = "output_e"
CONF_OUTPUT_D = "output_d"
CONF_OUTPUT_C = "output_c"
CONF_OUTPUT_G = "output_g"
CONF_REPEAT = "repeat"
CONF_LEAD_IN_MS = "lead_in_ms"
CONF_LEAD_OUT_MS = "lead_out_ms"

# C++ class references
FireCodeEffect = sequencer_ns.class_("FireCodeEffect", cg.Component)
StepSeqEffect = sequencer_ns.class_("StepSeqEffect", cg.Component)
MarchEffect = sequencer_ns.class_("MarchEffect", cg.Component)
ChimeSeqEffect = sequencer_ns.class_("ChimeSeqEffect", cg.Component)


# fire_code effect - for digit-based patterns like "4 4" or "3 4 2"
@register_binary_effect(
    "fire_code",
    FireCodeEffect,
    "Fire Code",
    {
        cv.Required(CONF_OUTPUT): cv.use_id(BinaryOutput),
        cv.Required(CONF_PATTERN): cv.string,
        cv.Optional(CONF_PULSE_MS, default=250): cv.positive_int,
        cv.Optional(CONF_GAP_MS): cv.positive_int,  # defaults to pulse_ms
        cv.Optional(CONF_GROUP_GAP_MS): cv.positive_int,  # defaults to gap_ms * 3
        cv.Optional(CONF_CYCLE_GAP_MS): cv.positive_int,  # defaults to group_gap_ms * 5
    },
)
async def fire_code_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    output = await cg.get_variable(config[CONF_OUTPUT])
    cg.add(var.set_output(output))
    cg.add(var.set_pattern(config[CONF_PATTERN]))

    pulse_ms = config[CONF_PULSE_MS]
    gap_ms = config.get(CONF_GAP_MS, pulse_ms)              # default: 1x pulse
    group_gap_ms = config.get(CONF_GROUP_GAP_MS, pulse_ms * 4)  # default: 4x pulse
    cycle_gap_ms = config.get(CONF_CYCLE_GAP_MS, 5000)      # default: 5 seconds

    cg.add(var.set_pulse_ms(pulse_ms))
    cg.add(var.set_gap_ms(gap_ms))
    cg.add(var.set_group_gap_ms(group_gap_ms))
    cg.add(var.set_cycle_gap_ms(cycle_gap_ms))
    return var


# step_seq effect - step sequencer patterns
# '.' = hit (ON), '-' = hold (continue), ' ' = rest (OFF)
@register_binary_effect(
    "step_seq",
    StepSeqEffect,
    "Step Sequencer",
    {
        cv.Required(CONF_OUTPUT): cv.use_id(BinaryOutput),
        cv.Required(CONF_PATTERN): cv.string,
        cv.Optional(CONF_STEP_MS, default=66): cv.positive_int,
    },
)
async def step_seq_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    output = await cg.get_variable(config[CONF_OUTPUT])
    cg.add(var.set_output(output))
    cg.add(var.set_pattern(config[CONF_PATTERN]))
    cg.add(var.set_step_ms(config[CONF_STEP_MS]))
    return var


# march effect - simple on/off toggle
@register_binary_effect(
    "march",
    MarchEffect,
    "March",
    {
        cv.Required(CONF_OUTPUT): cv.use_id(BinaryOutput),
        cv.Optional(CONF_ON_MS, default=500): cv.positive_int,
        cv.Optional(CONF_OFF_MS): cv.positive_int,  # defaults to on_ms
    },
)
async def march_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    output = await cg.get_variable(config[CONF_OUTPUT])
    cg.add(var.set_output(output))

    on_ms = config[CONF_ON_MS]
    off_ms = config.get(CONF_OFF_MS, on_ms)

    cg.add(var.set_on_ms(on_ms))
    cg.add(var.set_off_ms(off_ms))
    return var


# chime_seq effect - 4-bar chime sequencer with note names
# Pattern: E=E5, D=D5, C=C5, G=G4, *=all, space=rest, |=divider (ignored)
# Uses BinaryOutput for E (code_output) and Switch for D/C/G (shunt/reverse/trigger)
@register_binary_effect(
    "chime_seq",
    ChimeSeqEffect,
    "Chime Sequencer",
    {
        cv.Required(CONF_OUTPUT_E): cv.use_id(BinaryOutput),
        cv.Required(CONF_OUTPUT_D): cv.use_id(Switch),
        cv.Required(CONF_OUTPUT_C): cv.use_id(Switch),
        cv.Required(CONF_OUTPUT_G): cv.use_id(Switch),
        cv.Required(CONF_PATTERN): cv.string,
        cv.Optional(CONF_STEP_MS, default=500): cv.positive_int,
        cv.Optional(CONF_REPEAT, default=False): cv.boolean,
        cv.Optional(CONF_LEAD_IN_MS): cv.int_range(min=0),
        cv.Optional(CONF_LEAD_OUT_MS): cv.int_range(min=0),
    },
)
async def chime_seq_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])

    output_e = await cg.get_variable(config[CONF_OUTPUT_E])
    switch_d = await cg.get_variable(config[CONF_OUTPUT_D])
    switch_c = await cg.get_variable(config[CONF_OUTPUT_C])
    switch_g = await cg.get_variable(config[CONF_OUTPUT_G])

    cg.add(var.set_output_e(output_e))
    cg.add(var.set_switch_d(switch_d))
    cg.add(var.set_switch_c(switch_c))
    cg.add(var.set_switch_g(switch_g))
    cg.add(var.set_pattern(config[CONF_PATTERN]))
    cg.add(var.set_step_ms(config[CONF_STEP_MS]))
    cg.add(var.set_repeat(config[CONF_REPEAT]))
    step_ms = config[CONF_STEP_MS]
    lead_in_ms = config.get(CONF_LEAD_IN_MS, step_ms)
    lead_out_ms = config.get(CONF_LEAD_OUT_MS, step_ms)
    cg.add(var.set_lead_in_ms(lead_in_ms))
    cg.add(var.set_lead_out_ms(lead_out_ms))
    return var
