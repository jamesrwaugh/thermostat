#include <Noritake_VFD_GU7000.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

#include <driver_rs_wrapper.hpp>

#include "ThermoSaveData_bp.h"
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

const uint8_t gUnderlineImageData[5] = {
    0b0000'0001, 0b0000'0001, 0b0000'0001, 0b0000'0001, 0b0000'0001,
};

const uint8_t gBlankImageData[5] = {
    0b0000'0000, 0b0000'0000, 0b0000'0000, 0b0000'0000, 0b0000'0000,
};

DebounceState TimeButton;

class ScreenC {
 public:
  ScreenC(Noritake_VFD_GU7000& c) : S_(c) {
    S_.print("TEMP");
    S_.GU7000_setCursor(0, 8);
    S_.print("C");
    S_.GU7000_drawImage(1, 8, 5, 8, gUnderlineImageData);
  }

  ~ScreenC() {
    S_.GU7000_clearScreen();
  }

  void OnUpPressed() {
    if (Locked_) {
      AutoTwi t;
      if (CharIndex < CharSetLength - 1) {
        CharIndex += 1;
      } else {
        CharIndex = 0;
      }
      S_.print(CharDotWidth * CharIndex, 0, CharSet[CharIndex]);
    } else {
      if (CursorPosition < CharCount) {
        AutoTwi t;
        CursorPosition += 1;
        DrawIndicatorAtCursor();
      }
    }
  }

  void OnDownPressed() {
    if (Locked_) {
      AutoTwi t;
      if (CharIndex > 0) {
        CharIndex -= 1;
      } else {
        CharIndex = CharSetLength - 1;
      }
      S_.print(CharDotWidth * CharIndex, 0, CharSet[CharIndex]);
    } else {
      if (CursorPosition > 0) {
        AutoTwi t;
        CursorPosition -= 1;
        DrawIndicatorAtCursor();
      }
    }
  }

  void OnEnterPressed() {
    if (!Locked_) {
      AutoTwi t;
      Locked_ = true;
      DrawIndicatorAtCursor();
    } else {
      Locked_ = false;
    }
  }

  void OnHalfSecondPassed() {
    if (!Locked_) {
      ShowIndicator_ = !ShowIndicator_;
      DrawIndicatorAtCursor(ShowIndicator_);
    }
  }

  void DrawIndicatorAtCursor() {
    DrawIndicatorAtCursor(true);
  }

  void DrawIndicatorAtCursor(bool on) {
    AutoTwi t;
    S_.GU7000_drawImage(CursorPosition * CharDotWidth, CharDotHeight,
                        CharDotWidth, CharDotHeight,
                        on ? gUnderlineImageData : gBlankImageData);
  }

 private:
  Noritake_VFD_GU7000& S_;
  static constexpr uint8_t CharCount = 1;
  static constexpr uint8_t CharSetLength = 2;
  static constexpr uint8_t CharDotWidth = 5;
  static constexpr uint8_t CharDotHeight = 5;
  char Buffer[CharCount];
  const char CharSet[CharSetLength] = {'C', 'F'};
  bool ShowIndicator_{false};
  uint8_t CursorPosition{0};
  uint8_t CharIndex{0};
  bool Locked_{false};
};

int main() {
  setup();

  ThermoSaveData data;
  {
    data.magic = THERMO_STATE_DATA_MAGIC;
    data.set_point = 39;
    data.temp_display_unit = TEMP_UNIT_FREEDOM;
  }

  Noritake_VFD_GU7000 s(1);

  ScreenC c(s);

  {
    AutoTwi twi;
    s.GU7000_init();
  }

  {
    AutoTwi twi;
    c.EnterTemp();
  }

  while (1) {
    uint8_t pind = PIND;

    AutoTwi twi;
    _delay_ms(500);
    s.GU7000_setCursor(0, 8);
    s.print("C");
    _delay_ms(500);
    s.GU7000_drawImage(0, 7, 5, 8, gUnderlineImageData);

    if (TimeButton.Add(pind & _BV(PIND1))) {
      c.OnHalfSecondPassed();
    }
  }

  return 0;
}