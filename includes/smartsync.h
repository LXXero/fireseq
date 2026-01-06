#pragma once

// SmartSync command types
enum SmartSyncCmd {
  SMARTSYNC_NONE = 0,      // Sync only, no horn command
  SMARTSYNC_HORN_OFF = 1,  // 1001 - Horn Off
  SMARTSYNC_HORN_ON = 2,   // 1011 - Horn On
  SMARTSYNC_TEMPORAL = 3,  // 1000 - Temporal
  SMARTSYNC_MARCH = 4      // 1110 - March Time
};

// Command bit patterns (4 bits each)
static const bool SMARTSYNC_CMDS[5][4] = {
  {false, false, false, false},  // NONE: no command
  {true, false, false, true},    // HORN_OFF: 1001
  {true, false, true, true},     // HORN_ON: 1011
  {true, false, false, false},   // TEMPORAL: 1000
  {true, true, true, false}      // MARCH: 1110
};

class SmartSync {
public:
  uint32_t cycle_start = 0;
  uint32_t cmd_start_us = 0;
  uint8_t cycle_count = 0;
  bool in_cmd_window = false;

  // Timing constants per patent FIG. 2:
  // Bounce down (26): 0V→+V→0V (8ms) - longer per patent fig
  // Sync/command window (10ms): 8-15ms per patent, same length for both cycle types
  //   - non-cmd: 10ms 0V hold
  //   - cmd: 6ms pre-hold + 4ms command bits
  // Bounce up (28): +V→0V→+V (5ms) - shorter per patent fig
  // 980ms from sync to next command start
  static const uint32_t PRE_CMD_HOLD = 6;      // 6ms pre-hold (so 6+4=10ms matches non-cmd)
  static const uint32_t CMD_TIME = 4;          // 4 bits × 1ms each
  // Bounce down: 0V(3ms) +V(2ms) 0V(3ms) = 8ms
  // Sync/cmd: 10ms (same for both cycle types)
  // Bounce up: +V(2ms) 0V(1ms) +V(2ms) = 5ms
  // Total dropout: ~23ms, +V time: ~957ms for 980ms cycle
  static const uint32_t DROPOUT_TOTAL = 23;
  static const uint32_t CYCLE_MS = 980;
  static const uint8_t CMD_INTERVAL = 4;

  template<typename Shunt, typename Reverse, typename CodeOut>
  void run(bool initial_run, Shunt shunt, Reverse reverse, CodeOut code_output, SmartSyncCmd cmd) {
    if (initial_run) {
      cycle_start = millis();
      cycle_count = 0;
      shunt->turn_on();
      reverse->turn_off();
      code_output->turn_on();
    }

    uint32_t now = millis();
    uint32_t elapsed = now - cycle_start;

    if (elapsed >= CYCLE_MS) {
      cycle_start += CYCLE_MS;  // Add instead of reset to prevent drift
      elapsed -= CYCLE_MS;
      cycle_count = (cycle_count + 1) % CMD_INTERVAL;
      in_cmd_window = false;  // Reset for next cycle
    }

    // Cycle structure per FIG. 2 & 3:
    // 0-3ms:   0V (bounce down pulse 1)
    // 3-5ms:   +V (bounce back)
    // 5-8ms:   0V (bounce down pulse 2, settling)
    // 8-18ms:  10ms sync/command window (8-15ms per patent, using 10ms)
    //          - non-cmd: 10ms 0V hold
    //          - cmd: 6ms pre-hold + 4ms command bits
    // 18-20ms: +V (bounce up pulse 1)
    // 20-21ms: 0V (bounce back)
    // 21-23ms: +V (bounce up pulse 2, settling)
    // 23ms+:   +V charging

    bool is_cmd_cycle = (cmd != SMARTSYNC_NONE);  // Send command every cycle

    // Bounce down phase (0-8ms) - longer per patent fig
    if (elapsed < 3) {
      // First 0V pulse
      shunt->turn_off(); reverse->turn_off(); code_output->turn_off();
    } else if (elapsed < 5) {
      // Bounce back to +V
      shunt->turn_on(); reverse->turn_off(); code_output->turn_on();
    } else if (elapsed < 8) {
      // Second 0V pulse (settling)
      shunt->turn_off(); reverse->turn_off(); code_output->turn_off();
    }
    // Sync/command window (8-18ms) - 10ms total
    else if (elapsed < 18) {
      if (is_cmd_cycle && elapsed >= 14) {
        // Command bits (14-18ms) - last 4ms of window
        if (!in_cmd_window) {
          cmd_start_us = micros();
          in_cmd_window = true;
        }

        uint32_t cmd_elapsed_us = micros() - cmd_start_us;
        uint32_t bit_idx = cmd_elapsed_us / 1000;  // Which bit (0-3)

        code_output->turn_off();
        if (bit_idx < 4 && SMARTSYNC_CMDS[cmd][bit_idx]) {
          // Bit is 1: full 1ms -V pulse (1ms ±40µs per patent)
          shunt->turn_on(); reverse->turn_on();  // -V
        } else {
          // Bit is 0: full 1ms 0V
          shunt->turn_off(); reverse->turn_off(); // 0V
        }
      } else {
        // Pre-hold (cmd cycles 8-14ms) or full hold (non-cmd 8-18ms)
        shunt->turn_off(); reverse->turn_off(); code_output->turn_off();
      }
    }
    // Bounce up phase (18-23ms)
    else if (elapsed < 20) {
      shunt->turn_on(); reverse->turn_off(); code_output->turn_on();
    } else if (elapsed < 21) {
      shunt->turn_off(); reverse->turn_off(); code_output->turn_off();
    } else if (elapsed < 23) {
      shunt->turn_on(); reverse->turn_off(); code_output->turn_on();
    }
    // +V charging (23ms+)
    else {
      shunt->turn_on(); reverse->turn_off(); code_output->turn_on();
    }
  }
};
