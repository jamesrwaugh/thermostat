#pragma once

#include <stdint.h>

#include "twi_master.h"

struct AutoTwi final {
  static uint8_t instanceCount_;
  static constexpr uint8_t Gu7000SlaveAddr_ = 0x50;

  AutoTwi() {
    if (instanceCount_ == 0) {
      tw_master_setup_transmit(Gu7000SlaveAddr_);
    }
    instanceCount_ += 1;
  }

  ~AutoTwi() {
    instanceCount_ -= 1;
    if (instanceCount_ == 0) {
      tw_master_end_transmit();
    }
  }
};

struct DebounceState {
  uint8_t ZeroCount{0};
  bool IsSet{false};

  bool Add(bool sample) {
    bool changed = false;

    if (!sample) {
      if (ZeroCount < 2) {
        ZeroCount += 1;
      } else if (!IsSet) {
        IsSet = true;
        changed = true;
      }
    } else {
      IsSet = false;
      ZeroCount = 0;
    }

    return changed;
  };
};
