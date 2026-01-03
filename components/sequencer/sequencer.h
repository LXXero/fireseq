#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/light_effect.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace sequencer {

// FireCodeEffect - digit-based patterns like "4 4" or "3 4 2"
class FireCodeEffect : public light::LightEffect {
 public:
  explicit FireCodeEffect(const char *name) : LightEffect(name) {}

  void set_output(output::BinaryOutput *output) { output_ = output; }
  void set_pattern(const std::string &pattern) { pattern_ = pattern; }
  void set_pulse_ms(uint32_t ms) { pulse_ms_ = ms; }
  void set_gap_ms(uint32_t ms) { gap_ms_ = ms; }
  void set_group_gap_ms(uint32_t ms) { group_gap_ms_ = ms; }
  void set_cycle_gap_ms(uint32_t ms) { cycle_gap_ms_ = ms; }

  void start() override {
    this->parse_pattern_();
    ESP_LOGD("sequencer", "FireCodeEffect start: pattern='%s', groups=%d, pulse=%u, gap=%u, output=%s",
             pattern_.c_str(), num_groups_, pulse_ms_, gap_ms_, output_ ? "valid" : "NULL");
    this->last_run_ = millis();
    this->current_group_ = 0;
    this->current_pulse_ = 0;
    // Start with initial gap (1 unit of gap_ms before first pulse)
    this->phase_ = PHASE_INITIAL_GAP;
    if (output_) output_->turn_off();
  }

  void apply() override {
    if (output_ == nullptr) {
      ESP_LOGW("sequencer", "FireCodeEffect: output is null!");
      return;
    }

    uint32_t now = millis();

    switch (phase_) {
      case PHASE_INITIAL_GAP:
        output_->turn_off();
        if (now - last_run_ >= gap_ms_) {
          last_run_ = now;
          phase_ = PHASE_PULSE_ON;
        }
        break;

      case PHASE_PULSE_ON:
        output_->turn_on();
        if (now - last_run_ >= pulse_ms_) {
          last_run_ = now;
          current_pulse_++;
          if (current_pulse_ >= groups_[current_group_]) {
            current_pulse_ = 0;
            current_group_++;
            if (current_group_ >= num_groups_) {
              phase_ = PHASE_CYCLE_GAP;
            } else {
              phase_ = PHASE_GROUP_GAP;
            }
          } else {
            phase_ = PHASE_PULSE_OFF;
          }
        }
        break;

      case PHASE_PULSE_OFF:
        output_->turn_off();
        if (now - last_run_ >= gap_ms_) {
          last_run_ = now;
          phase_ = PHASE_PULSE_ON;
        }
        break;

      case PHASE_GROUP_GAP:
        output_->turn_off();
        if (now - last_run_ >= group_gap_ms_) {
          last_run_ = now;
          phase_ = PHASE_PULSE_ON;
        }
        break;

      case PHASE_CYCLE_GAP:
        output_->turn_off();
        // Subtract gap_ms since we already had initial gap at start
        if (now - last_run_ >= (cycle_gap_ms_ - gap_ms_)) {
          last_run_ = now;
          current_group_ = 0;
          current_pulse_ = 0;
          phase_ = PHASE_INITIAL_GAP;
        }
        break;
    }

  }

 protected:
  void parse_pattern_() {
    num_groups_ = 0;
    for (size_t i = 0; i < pattern_.size() && num_groups_ < 10; i++) {
      char c = pattern_[i];
      if (c >= '1' && c <= '9') {
        groups_[num_groups_++] = c - '0';
      }
    }
    if (num_groups_ == 0) {
      groups_[0] = 1;
      num_groups_ = 1;
    }
  }

  output::BinaryOutput *output_{nullptr};
  std::string pattern_;
  uint32_t pulse_ms_{250};
  uint32_t gap_ms_{250};
  uint32_t group_gap_ms_{750};
  uint32_t cycle_gap_ms_{5000};

  uint32_t last_run_{0};

  enum Phase { PHASE_INITIAL_GAP, PHASE_PULSE_ON, PHASE_PULSE_OFF, PHASE_GROUP_GAP, PHASE_CYCLE_GAP };
  Phase phase_{PHASE_INITIAL_GAP};

  int groups_[10];
  int num_groups_{0};
  int current_group_{0};
  int current_pulse_{0};
};


// StepSeqEffect - step sequencer patterns
// '.' = hit (turn ON), '-' = hold (continue), ' ' = rest (turn OFF)
class StepSeqEffect : public light::LightEffect {
 public:
  explicit StepSeqEffect(const char *name) : LightEffect(name) {}

  void set_output(output::BinaryOutput *output) { output_ = output; }
  void set_pattern(const std::string &pattern) { pattern_ = pattern; }
  void set_step_ms(uint32_t ms) { step_ms_ = ms; }

  void start() override {
    ESP_LOGD("sequencer", "StepSeqEffect start: pattern='%s', step_ms=%u", pattern_.c_str(), step_ms_);
    this->pos_ = 0;
    this->last_step_ = millis();
    // Start with initial rest (like other effects)
    if (output_) output_->turn_off();
  }

  void apply() override {
    if (output_ == nullptr) return;

    uint32_t now = millis();

    // Time for next step?
    if (now - last_step_ >= step_ms_) {
      last_step_ = now;

      if (pattern_.empty()) return;

      char c = pattern_[pos_];

      if (c == '.') {
        // Hit - turn ON
        output_->turn_on();
      } else if (c == ' ') {
        // Rest - turn OFF
        output_->turn_off();
      }
      // '-' = hold - do nothing, continue previous state

      pos_++;
      if (pos_ >= pattern_.size()) {
        pos_ = 0;
      }
    }
  }

 protected:
  output::BinaryOutput *output_{nullptr};
  std::string pattern_;
  uint32_t step_ms_{66};

  size_t pos_{0};
  uint32_t last_step_{0};
};


// ChimeSeqEffect - 4-bar chime sequencer with note names
// Pattern chars: E=E5, D=D5, C=C5, G=G4, *=all, space=rest, |=ignored (divider)
// Consistent with step_seq: space is rest, letters are hits
// Uses BinaryOutput for E (code_output) and Switch for D/C/G (shunt/reverse/trigger)
class ChimeSeqEffect : public light::LightEffect {
 public:
  explicit ChimeSeqEffect(const char *name) : LightEffect(name) {}

  void set_output_e(output::BinaryOutput *output) { output_e_ = output; }
  void set_switch_d(switch_::Switch *sw) { switch_d_ = sw; }
  void set_switch_c(switch_::Switch *sw) { switch_c_ = sw; }
  void set_switch_g(switch_::Switch *sw) { switch_g_ = sw; }
  void set_pattern(const std::string &pattern) { pattern_ = pattern; }
  void set_step_ms(uint32_t ms) { step_ms_ = ms; }
  void set_repeat(bool repeat) { repeat_ = repeat; }
  void set_lead_in_ms(uint32_t ms) { lead_in_ms_ = ms; }
  void set_lead_out_ms(uint32_t ms) { lead_out_ms_ = ms; }

  void start() override {
    // Pre-process pattern to remove dividers only
    clean_pattern_.clear();
    for (char c : pattern_) {
      if (c != '|') {
        clean_pattern_ += c;
      }
    }
    ESP_LOGD("sequencer", "ChimeSeqEffect start: pattern='%s', step_ms=%u", pattern_.c_str(), step_ms_);
    this->finished_ = false;
    this->lead_out_active_ = false;
    this->suppress_initial_light_on_ = true;

    // Always start with everything OFF (like old lambda's state 0 on initial_run)
    if (output_e_) output_e_->turn_off();
    if (switch_d_) switch_d_->turn_off();
    if (switch_c_) switch_c_->turn_off();
    if (switch_g_) switch_g_->turn_off();

    // Start from position 0; lead-in is expressed as rest steps
    this->pos_ = 0;
    this->lead_in_remaining_ = (lead_in_ms_ + step_ms_ - 1) / step_ms_;
    this->last_step_ = millis() - step_ms_;
  }

  void stop() override {
    // Ensure all outputs are OFF when effect stops
    all_off_();
  }

  void apply() override {
    if (finished_) return;

    uint32_t now = millis();

    if (lead_out_active_) {
      if (now < lead_out_end_) {
        return;
      }
      all_off_();
      finished_ = true;
      auto call = this->state_->turn_off();
      call.perform();
      return;
    }

    if (suppress_initial_light_on_) {
      suppress_initial_light_on_ = false;
      if (this->state_ != nullptr && this->state_->current_values.is_on()) {
        this->state_->current_values.set_state(false);
      }
    }

    if (lead_in_remaining_ > 0) {
      // Keep outputs OFF immediately during lead-in, even before the first step ticks.
      all_off_();
    }

    // Not time for next step yet
    if (now - last_step_ < step_ms_) {
      return;
    }

    // Time for next step - use addition to prevent drift
    last_step_ += step_ms_;

    if (lead_in_remaining_ > 0) {
      // Lead-in is a silent rest step before the first note.
      all_off_();
      lead_in_remaining_--;
      return;
    }

    if (clean_pattern_.empty()) return;

    char c = clean_pattern_[pos_];
    bool is_all = (c == '*' || c == 'A' || c == 'a');

    // Each output turns ON if activated, or OFF only if currently on
    if (c == 'E' || c == 'e' || is_all) {
      if (output_e_) output_e_->turn_on();
    } else {
      if (output_e_) output_e_->turn_off();
    }

    if (c == 'D' || c == 'd' || is_all) {
      if (switch_d_) switch_d_->turn_on();
    } else if (switch_d_ && switch_d_->state) {
      switch_d_->turn_off();
    }

    if (c == 'C' || c == 'c' || is_all) {
      if (switch_c_) switch_c_->turn_on();
    } else if (switch_c_ && switch_c_->state) {
      switch_c_->turn_off();
    }

    if (c == 'G' || c == 'g' || is_all) {
      if (switch_g_) switch_g_->turn_on();
    } else if (switch_g_ && switch_g_->state) {
      switch_g_->turn_off();
    }

    pos_++;
    if (pos_ >= clean_pattern_.size()) {
      if (repeat_) {
        pos_ = 0;
      } else {
        if (lead_out_ms_ > 0) {
          lead_out_active_ = true;
          lead_out_end_ = now + lead_out_ms_;
        } else {
          all_off_();
          finished_ = true;
          auto call = this->state_->turn_off();
          call.perform();
        }
      }
    }
  }

 protected:
  void all_off_() {
    if (output_e_) output_e_->turn_off();
    if (switch_d_) switch_d_->turn_off();
    if (switch_c_) switch_c_->turn_off();
    if (switch_g_) switch_g_->turn_off();
  }

  output::BinaryOutput *output_e_{nullptr};
  switch_::Switch *switch_d_{nullptr};
  switch_::Switch *switch_c_{nullptr};
  switch_::Switch *switch_g_{nullptr};
  std::string pattern_;
  std::string clean_pattern_;
  uint32_t step_ms_{500};
  bool repeat_{false};
  uint32_t lead_in_ms_{0};
  size_t lead_in_remaining_{0};
  uint32_t lead_out_ms_{0};
  uint32_t lead_out_end_{0};
  bool lead_out_active_{false};

  size_t pos_{0};
  uint32_t last_step_{0};
  bool finished_{false};
  bool suppress_initial_light_on_{false};
};


// MarchEffect - simple on/off toggle
class MarchEffect : public light::LightEffect {
 public:
  explicit MarchEffect(const char *name) : LightEffect(name) {}

  void set_output(output::BinaryOutput *output) { output_ = output; }
  void set_on_ms(uint32_t ms) { on_ms_ = ms; }
  void set_off_ms(uint32_t ms) { off_ms_ = ms; }

  void start() override {
    ESP_LOGD("sequencer", "MarchEffect start: on_ms=%u, off_ms=%u, output=%s",
             on_ms_, off_ms_, output_ ? "valid" : "NULL");
    // Start OFF with initial gap (matches old lambda behavior)
    this->is_on_ = false;
    this->last_toggle_ = millis();
    if (output_) output_->turn_off();
  }

  void apply() override {
    if (output_ == nullptr) return;

    uint32_t now = millis();
    uint32_t duration = is_on_ ? on_ms_ : off_ms_;

    if (now - last_toggle_ >= duration) {
      last_toggle_ = now;
      is_on_ = !is_on_;
      if (is_on_) {
        output_->turn_on();
      } else {
        output_->turn_off();
      }
    }
  }

 protected:
  output::BinaryOutput *output_{nullptr};
  uint32_t on_ms_{500};
  uint32_t off_ms_{500};
  bool is_on_{true};
  uint32_t last_toggle_{0};
};

}  // namespace sequencer
}  // namespace esphome
