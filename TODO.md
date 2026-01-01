# FireSeq TODO

## Howe Decoder Enhancements

### Display & Integration
- [ ] OLED display integration - show active stations with round progress
- [ ] Home Assistant integration - expose stations as sensors with attributes
  - Auto-discover stations as entities
  - Track round_count, prefix, last_seen, status
  - Enable automations on station events

### Alarm vs Reset Detection
- [ ] Detect alarm vs reset based on round count
  - 5 rounds of a code = ALARM
  - 1 round of same code = RESET
  - Would be useful for HA entity status

### Station Type Handling
- [ ] V### station support - single round stations (V133, V122, etc.)
  - Prefix 5 = alarm
  - Prefix 6 = reset
  - No multi-round expected

- [ ] Continuous station detection
  - Some stations run continuously (always transmitting)
  - Need to detect "only continuous stations running" vs actual alarms
  - Maybe mark transmission complete for non-continuous when they stop?

- [ ] Watchman mode support
  - 1 round, usually prefix 1
  - Some fire alarms also prefix 1 but multi-round
  - Differentiate by round count?

### Prefix Reference
```
1 = Watchman / Fire (context-dependent)
2 = Fire Alarm / Watchman (depends on transmitter type)
3 = Supervisory / Waterflow / Presignal
5 = Alarm (V### stations)
6 = Reset (V### stations)
```

### Transmitter Types
```
B  = Howe-in-a-can with terminal blocks
     - Combo Fire & Watch boxes: no 5-round capability, prefixes may be flipped vs BJ (TBD)
     - Fire-only boxes: some have wheel mechanism for 5 rounds
BX = Howe-in-a-can with molex (Federal Signal era)
BJ = Horizontal mount with banana plug pins
     Combination Fire & Watch versions have 5-round capability
```

## Notes

- 2-1-2-1-2: 1930s phone box (Type B)
- 2-2-1-2: 1930s round station (Type B)

