#include <Noritake_VFD_GU7000.h>
#include <avr/io.h>
#include <util/delay.h>

#include <driver_rs_wrapper.hpp>

#include "ThermoSaveData_bp.h"
#include "screen.hpp"
#include "utils.hpp"

// 112 x 16
// Temp Display Unit
// Date Set
// Time Set
enum class Screen : uint8_t {
  TempDisplayUnit = 0,
  DateSet = 1,
  TimeSet = 2,
};

uint8_t AutoTwi::instanceCount_ = 0;

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
  // -> Input pullup
  DDRD &= ~_BV(DDD2);
  DDRD &= ~_BV(DDD3);
  DDRD &= ~_BV(DDD4);
  DDRD &= ~_BV(DDD5);
  DDRD &= ~_BV(DDD6);
  DDRD &= ~_BV(DDD7);
  PORTD |= _BV(PORTD2) | _BV(PORTD3) | _BV(PORTD4) | _BV(PORTD5) | _BV(PORTD6) |
           _BV(PORTD7);
}

int main() {
  setup();

  ThermoSaveData data;
  data.magic = THERMO_STATE_DATA_MAGIC;
  data.set_point = 39;
  data.temp_display_unit = TEMP_UNIT_FREEDOM;

  ScreenBoxStorage Boxes[5];

  Noritake_VFD_GU7000 gu7k(1);

  ScreenBox::Screen_ = &gu7k;
  ScreenC::Screen_ = &gu7k;

  {
    AutoTwi twi;
    gu7k.GU7000_init();
  }

  ScreenC tempScreen("TEMP", data, Boxes);

  DebounceState UpButton;
  DebounceState DownButton;
  DebounceState SelectButton;
  DebounceState TimeButton;

  uint8_t someCount = 0;

  while (1) {
    uint8_t pind = PIND;

    if (++someCount > 50) {
      someCount = 0;

      if (UpButton.Add(pind & _BV(PIND0))) {
        tempScreen.OnUpPressed();
      } else if (DownButton.Add(pind & _BV(PIND1))) {
        tempScreen.OnDownPressed();
      } else if (SelectButton.Add(pind & _BV(PIND2))) {
        tempScreen.OnSelectPressed();
      } else if (TimeButton.Add(pind & _BV(PIND3))) {
        tempScreen.OnHalfSecondPassed();
      }
    }
  }

  return 0;
}