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

struct ScreenCBox {
  static constexpr uint8_t CharDotWidth = 5;
  static constexpr uint8_t CharDotHeight = 7;
  static Noritake_VFD_GU7000* Screen_;
  const char* CharSet;
  const uint8_t CharSetLength;
  const uint8_t xPositionChars;
  uint8_t CharIndex{0};

  ScreenCBox(uint8_t xPositionChars, const char* set, uint8_t count)
      : CharSet{set}, CharSetLength{count}, xPositionChars{xPositionChars} {}

  void Up() {
    if (CharIndex < CharSetLength - 1) {
      CharIndex += 1;
    } else {
      CharIndex = 0;
    }
    Draw();
  }

  void Down() {
    if (CharIndex > 0) {
      CharIndex -= 1;
    } else {
      CharIndex = CharSetLength - 1;
    }
    Draw();
  }

  void Draw() const {
    AutoTwi t;
    Screen_->print(xPositionChars * CharDotWidth, 0, CharSet[CharIndex]);
  }

  void DrawIndicator() const {
    DrawIndicator(true);
  }

  void DrawIndicator(bool on) const {
    AutoTwi t;
    Screen_->GU7000_drawImage(xPositionChars * CharDotWidth, CharDotHeight,
                              CharDotWidth, CharDotHeight,
                              on ? gUnderlineImageData : gBlankImageData);
  }
};

Noritake_VFD_GU7000* ScreenCBox::Screen_ = nullptr;

ScreenCBox TempScreenBoxes[1] = {
    ScreenCBox(0, "CF", 2),
};

class ScreenC {
 public:
  ScreenC(Noritake_VFD_GU7000& s, const char* title, ScreenCBox* boxes,
          uint8_t boxesCount)
      : S_(s), Chars{boxes}, CharCount{boxesCount} {
    AutoTwi t;
    S_.print(title);
    S_.GU7000_setCursor(0, 8);
    for (uint8_t i = 0; i < CharCount; ++i) {
      Chars[i].Draw();
    }
  }

  ~ScreenC() {
    AutoTwi t;
    S_.GU7000_clearScreen();
  }

  void OnUpPressed() {
    if (Locked_) {
      Chars[CursorPosition].Up();
    } else {
      if (CursorPosition < CharCount - 1) {
        CursorPosition += 1;
        Chars[CursorPosition].DrawIndicator();
      }
    }
  }

  void OnDownPressed() {
    if (Locked_) {
      Chars[CursorPosition].Down();
    } else {
      if (CursorPosition > 0) {
        CursorPosition -= 1;
        Chars[CursorPosition].DrawIndicator();
      }
    }
  }

  void OnEnterPressed() {
    if (!Locked_) {
      Locked_ = true;
      Chars[CursorPosition].DrawIndicator();
    } else {
      Locked_ = false;
    }
  }

  void OnHalfSecondPassed() {
    if (!Locked_) {
      ShowIndicator_ = !ShowIndicator_;
      Chars[CursorPosition].DrawIndicator(ShowIndicator_);
    }
  }

 private:
  Noritake_VFD_GU7000& S_;
  ScreenCBox* const Chars;
  const uint8_t CharCount;
  bool ShowIndicator_{false};
  uint8_t CursorPosition{0};
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

  ScreenCBox::Screen_ = &s;

  ScreenC c(s, "TEMP", TempScreenBoxes, 1);

  {
    AutoTwi twi;
    s.GU7000_init();
  }

  {
    AutoTwi twi;
    // c.EnterTemp();
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