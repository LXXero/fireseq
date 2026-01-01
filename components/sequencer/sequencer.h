#pragma once

#include "esphome/core/component.h"
#include "esphome/components/light/light_effect.h"
#include "esphome/components/output/binary_output.h"

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
