#pragma once

#include <stdint.h>

#include "twi_master.h"

struct AutoTwi final {
  static constexpr uint8_t Gu7000SlaveAddr_ = 0x50;

  AutoTwi() {
    tw_master_setup_transmit(Gu7000SlaveAddr_);
  }

  ~AutoTwi() {
    tw_master_end_transmit();
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

void setup() {
  // initialize twi prescaler and bit rate (250k)
  // Set prescaler value of 1
  TWSR &= ~_BV(TWPS0);
  TWSR &= ~_BV(TWPS1);
  TWBR = ((F_CPU / 250000) - 16) / 2;

  // Screen Busy
  // -> Input
  DDRB &= ~_BV(DDB1);
  PORTB &= ~_BV(PORTB1);

  // Buttons
  DDRD &= ~_BV(DDD2);
  DDRD &= ~_BV(DDD3);
  DDRD &= ~_BV(DDD4);
  DDRD &= ~_BV(DDD5);
  DDRD &= ~_BV(DDD6);
  DDRD &= ~_BV(DDD7);
  PORTD |= _BV(PORTD2) | _BV(PORTD3) | _BV(PORTD4) | _BV(PORTD5) | _BV(PORTD6) |
           _BV(PORTD7);
}