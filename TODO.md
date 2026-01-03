# FireSeq TODO

## Completed

### Howe Decoder Improvements
- [x] b_combo prefix 2 detection: reset (if fire active) vs watchman
- [x] Added "reset" mode with proper state handling
- [x] Continuous fire station handling (immediate FIRE, CLEAR on bus stop)
- [x] Howe Follower effect - output mirrors incoming signal
- [x] Round-robin detection improvements
- [x] Per-station round tracking

### Chime Sequencer
- [x] `chime_seq` effect for 4-bar xylophone chimes (E5, D5, C5, G4)
- [x] Pattern syntax: note names (E/D/C/G), * for all, space for rest, | for dividers
- [x] Non-repeating melodies with auto-off on completion
- [x] 14 doorbell melodies (Westminster, Turkey, Camptown, etc.)

---

## Station Database (Next Up)

### Data Model Design
Station ID = digits AFTER prefix (e.g., "1-4-5" not "2-1-4-5")
Prefix indicates MODE, not part of station identity.

**Station Types & Prefix Mappings:**
```
bj_combo             = 1:watchman, 2:fire
bj_combo_presignal   = 1:watchman, 2:presignal, 3:general_alarm
bj_fire              = 1:fire (5 rounds alarm, 1 reset)
b_combo              = 1:fire, 2:watchman (flipped vs BJ)
b_fire_presignal     = 2:presignal, 3:general_alarm
waterflow            = 3:waterflow (5 rounds alarm, 1 reset)
v_station            = 5:alarm, 6:reset (single round each)
continuous           = special handling TBD
```

**Round Count Logic (most modes):**
- 5 rounds = ALARM
- 1 round = RESET
- 6 rounds = ALARM + RESET (reset while running)

**Exceptions:**
- Watchman: always 1 round (no alarm/reset)
- V-stations: always 1 round, prefix determines alarm vs reset
- Continuous: always running, special handling

**Presignal Behavior:**
- Prefix 2 = presignal (early warning, limited response)
- Prefix 3 = general alarm (full evacuation)
- If both seen from same station, prefix 3 takes precedence

### Implementation Plan
- [x] Create includes/howe_stations.yaml with globals for station maps
- [x] Define station types with prefix->mode mappings
- [x] Macro for easy station definition: ADD_STATION(id, name, loc, type)
- [x] Override macro for non-standard prefix behavior
- [x] Update decoder to parse prefix vs station_id separately
- [x] Look up station type and determine mode from prefix
- [x] Interpret round count based on mode
- [x] Log enriched output: "[station] name (mode) - STATUS"

---

## Howe Decoder Enhancements

### Display & Integration
- [ ] OLED display integration - show active stations with round progress
- [ ] Home Assistant integration - expose stations as sensors with attributes
  - Auto-discover stations as entities
  - Track round_count, prefix, last_seen, status
  - Enable automations on station events

### Continuous Station Detection
- [x] Continuous fire state handled (immediate FIRE, CLEAR on bus stop)
- [x] Detect "only continuous stations running" vs actual alarms

### Prefix Reference
```
1 = Watchman / Fire (context-dependent on station type)
2 = Fire / Presignal / Watchman (context-dependent)
3 = General Alarm / Waterflow / Supervisory (context-dependent)
5 = Alarm (V### stations only)
6 = Reset (V### stations only)
```

### Transmitter Types
```
B  = Howe-in-a-can with terminal blocks
     - Combo Fire & Watch boxes: no 5-round capability, prefixes flipped vs BJ
     - Fire-only boxes: some have wheel mechanism for 5 rounds
BX = Howe-in-a-can with molex (Federal Signal era)
BJ = Horizontal mount with banana plug pins
     - Combination Fire & Watch versions have 5-round capability
```

## Notes

- 2-1-2-1-2: 1930s phone box (Type B)
- 2-2-1-2: 1930s round station (Type B)

