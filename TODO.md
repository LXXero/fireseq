# FireSeq TODO

## Completed

### Howe Decoder Improvements
- [x] b_combo prefix 2 detection: reset (if fire active) vs watchman
- [x] Added "reset" mode with proper state handling
- [x] Continuous fire station handling (immediate FIRE, CLEAR on bus stop)
- [x] Howe Follower effect - output mirrors incoming signal
- [x] Round-robin detection improvements
- [x] Per-station round tracking
- [x] Predictive state logic - trigger states early when unambiguous
  - V-stations: immediate state from prefix (5=FIRE, 6=CLEAR)
  - Watchman: immediate trigger (always 1 round)
  - General Alarm: immediate trigger (requires break glass + key turn)
  - Round 2+: predictive fire/presignal/waterflow (resets are always 1 round)
- [x] Interim summary - summarize stations that drop out mid-transmission
  - Detects when station misses full rotation while others continue
  - Applies full 5-round/1-round logic to interim summaries

### Chime Sequencer
- [x] `chime_seq` effect for 4-bar xylophone chimes (E5, D5, C5, G4)
- [x] Pattern syntax: note names (E/D/C/G), * for all, space for rest, | for dividers
- [x] Non-repeating melodies with auto-off on completion
- [x] 14 doorbell melodies (Westminster, Turkey, Camptown, etc.)

### Architecture
- [x] Split howe_panel.yaml from howe_decoder.yaml
  - howe_panel.yaml includes howe_decoder.yaml (panel users get both)
  - howe_decoder.yaml standalone for users without panel integration
  - Clean separation of concerns

### Station Database
- [x] Station database implemented in howe_stations.yaml
- [x] Data model with prefix->mode mappings per station type
- [x] Decoder integration with enriched logging

#### Data Model Reference
Station ID = digits AFTER prefix (e.g., "1-4-5" not "2-1-4-5")
Prefix indicates MODE, not part of station identity.

**Station Types & Prefix Mappings:**
```
bj_combo             = 1:watchman, 2:fire
bj_combo_presignal   = 1:watchman, 2:presignal, 3:general_alarm
bj_fire              = 1:fire (5 rounds alarm, 1 reset)
b_combo              = 1:fire (continuous), 2:watchman OR reset (if fire active)
b_fire               = 1:fire (continuous)
b_fire_presignal     = 2:presignal, 3:general_alarm
waterflow            = 3:waterflow (5 rounds alarm, 1 reset)
v_station            = 5:alarm, 6:reset (single round each)
```

**Round Count Logic (most modes):**
- 5 rounds = ALARM
- 1 round = RESET
- 6 rounds = ALARM + RESET (reset while running)

**Exceptions:**
- Watchman: always 1 round (no alarm/reset distinction)
- V-stations: always 1 round, prefix determines alarm (5) vs reset (6)
- Continuous (B-types with prefix 1): transmit continuously while active
  - Immediate FIRE state on first round
  - CLEAR state when bus stops (detected via interim summary)
  - No round counting - presence = alarm, absence = clear

**Presignal Behavior:**
- Prefix 2 = presignal (early warning, limited response)
- Prefix 3 = general alarm (full evacuation)
- If both seen from same station, prefix 3 takes precedence

---

## Open Items

### Display & Integration
- [ ] OLED display integration - show active stations with round progress
- [ ] Howe display mode - scrollable list of recent incoming codes
  - Switch display into "Howe mode" during transmission
  - Show previous codes with their status
  - Scroll through history
- [ ] Status indicator on display header
  - Shorten "FireSeq" or rework first line to fit status
  - Show system state: OK / TBL / ALM / WM / FIRE
  - Priority: FIRE > ALM > TBL > WM > OK
  - Maybe invert for alarm states?
- [ ] Home Assistant integration - expose stations as sensors with attributes
  - Auto-discover stations as entities
  - Track round_count, prefix, last_seen, status
  - Enable automations on station events

### Panel Output Integration (Simplex 2001)
- [x] Alarm pulse output - trigger panel on Howe alarm
  - PC817 optocoupler + 1N4148 diode for D-bus isolation
  - **TESTED: 400ms pulse required for Simplex 2001 alarm activation**
  - GPIO pulse when any station goes to FIRE/ALM/GENERAL_ALARM
- [x] Steady 24V outputs for alarm/presignal level switching
  - MIC2981 8-channel high-side driver installed
  - GPIO22 (alarm), GPIO21 (presignal) active
  - L298 H-bridge retained for SmartSync (polarity inversion, tight timing)
- [x] Panel switches trigger 400ms pulse on turn_on
  - Alarm and presignal switches auto-pulse when activated
  - Works from both Howe decoder AND manual HA toggle
- [x] Remote Ack output (GPIO4) - 400ms pulse
- [x] Remote Reset output (GPIO26) - 2s pulse
- [x] Trouble Reset output (GPIO2) - 400ms pulse after main reset
  - GPIO2 used (held LOW at boot, safe for MIC2981)
- [ ] Oscillator-triggered Howe Follower workflow
  - Oscillator start → trigger pulse output + switch to "Howe Follower" effect
  - Code output follows incoming signal during transmission
  - Transmission complete → determine final result (presignal/alarm)
  - Switch code output back to previous effect (before Howe Follower)
  - Trigger presignal or alarm output based on result
- [ ] Trouble output - TBD what counts as "trouble"
  - Watchman-only stations? Supervisory?

---

## Reference

### Prefix Reference
```
1 = Watchman (BJ) / Fire (B-types, continuous)
2 = Fire (BJ) / Presignal / Watchman (B) / Reset (B, if fire active)
3 = General Alarm / Waterflow
5 = Alarm (V-stations only, single round)
6 = Reset (V-stations only, single round)
```

### Transmitter Types
```
B  = Howe-in-a-can with terminal blocks (continuous fire, no round wheel)
     - Combo: prefix 1=fire (continuous), prefix 2=watchman or reset
     - Fire-only: prefix 1=fire (continuous)
     - Fire-Presignal: prefix 2=presignal, prefix 3=general alarm
     - Prefixes flipped vs BJ types
BX = Howe-in-a-can with molex (Federal Signal era)
BJ = Horizontal mount with banana plug pins (5-round capable)
     - Combo: prefix 1=watchman, prefix 2=fire (5 rounds alarm, 1 reset)
     - Fire-only: prefix 1=fire (5 rounds alarm, 1 reset)
     - Presignal variants add prefix 2=presignal, prefix 3=general alarm
```

## Notes

- 2-1-2-1-2: 1930s phone box (Type B)
- 2-2-1-2: 1930s round station (Type B)

