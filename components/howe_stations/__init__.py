import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@LXXero"]
MULTI_CONF = False

# Station Types (behavioral names)
#
# combo_5round      - Prefix 1=watchman, 2=fire (5-round). Usually BJ stations.
# combo_continuous  - Prefix 1=fire (continuous), 2=watchman/reset. Usually B stations.
# combo_presignal   - Prefix 1=watchman, 2=presignal, 3=GA. Usually BJ stations.
# fire_5round       - Prefix 1/2=fire (5-round). Can be B or BJ stations.
# fire_continuous   - Prefix 1=fire (continuous). Usually B stations ("Fire Loaf", etc).
# fire_presignal    - Prefix 2=presignal, 3=GA. Can be B or BJ stations.
# fire_v            - Prefix 5=alarm, 6=reset (single round). Usually B stations (V133, etc).
# waterflow         - Prefix 3=waterflow. Usually B stations (waterflow/tamper switches).
#
STATION_TYPES = [
    "combo_5round",
    "combo_continuous",
    "combo_presignal",
    "fire_5round",
    "fire_continuous",
    "fire_presignal",
    "fire_v",
    "waterflow",
]

# Valid modes for overrides
OVERRIDE_MODES = [
    "fire",
    "watchman",
    "presignal",
    "general_alarm",
    "waterflow",
    "v_alarm",
    "v_reset",
]

# Default continuous flag per type (False if not listed)
TYPE_CONTINUOUS_DEFAULT = {
    "combo_continuous": True,
    "fire_continuous": True,
}

# Default tamper flag per type (False if not listed)
TYPE_TAMPER_DEFAULT = {
    "waterflow": True,
}

# Schema for a single station
STATION_SCHEMA = cv.Schema({
    cv.Required("name"): cv.string,
    cv.Optional("location", default=""): cv.string,
    cv.Required("type"): cv.one_of(*STATION_TYPES, lower=True),
    cv.Optional("continuous"): cv.boolean,
    cv.Optional("tamper"): cv.boolean,
    cv.Optional("overrides"): cv.Schema({
        cv.int_range(min=1, max=9): cv.one_of(*OVERRIDE_MODES, lower=True),
    }),
})

# Main config schema - station IDs map directly to station config
CONFIG_SCHEMA = cv.Schema({
    cv.string: STATION_SCHEMA,
})


async def to_code(config):
    """Generate C++ code that initializes station globals at boot."""
    # Build initialization statements
    init_lines = []
    station_count = 0

    for station_id, station_config in config.items():
        name = station_config["name"]
        location = station_config.get("location", "")
        station_type = station_config["type"]

        # Get continuous/tamper with defaults
        continuous = station_config.get("continuous", TYPE_CONTINUOUS_DEFAULT.get(station_type, False))
        tamper = station_config.get("tamper", TYPE_TAMPER_DEFAULT.get(station_type, False))

        # Escape strings for C++
        name_esc = name.replace('\\', '\\\\').replace('"', '\\"')
        location_esc = location.replace('\\', '\\\\').replace('"', '\\"')

        init_lines.append(f'id(station_names)["{station_id}"] = "{name_esc}";')
        init_lines.append(f'id(station_locations)["{station_id}"] = "{location_esc}";')
        init_lines.append(f'id(station_types)["{station_id}"] = "{station_type}";')
        init_lines.append(f'id(station_continuous)["{station_id}"] = {"true" if continuous else "false"};')
        init_lines.append(f'id(station_tamper)["{station_id}"] = {"true" if tamper else "false"};')

        # Add overrides
        overrides = station_config.get("overrides", {})
        for prefix, mode in overrides.items():
            init_lines.append(f'id(station_prefix_overrides)["{station_id}:{prefix}"] = "{mode}";')

        station_count += 1

    init_code = '\n      '.join(init_lines)

    # Define component class inside setup() so globals are in scope
    # (local classes are valid C++ and work for our simple use case)
    cg.add(cg.RawStatement(f'''
  class HoweStationsInit : public Component {{
   public:
    float get_setup_priority() const override {{ return -200.0f; }}
    void setup() override {{
      {init_code}
      ESP_LOGI("howe_stations", "Initialized {station_count} stations");
    }}
  }};
  App.register_component(new HoweStationsInit());
'''))
