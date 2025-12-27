#include "screen.hpp"

#include <Noritake_VFD_GU7000.h>
#include <etl/placement_new.h>
#include <stdint.h>

#include "ThermoSaveData_bp.h"
#include "utils.hpp"

const uint8_t gUnderlineImageData[5] = {
    0b0000'0001, 0b0000'0001, 0b0000'0001, 0b0000'0001, 0b0000'0001,
};

const uint8_t gBlankImageData[5] = {
    0b0000'0000, 0b0000'0000, 0b0000'0000, 0b0000'0000, 0b0000'0000,
};

// ================================================================ //

Noritake_VFD_GU7000* ScreenBox::Screen_ = nullptr;

ScreenBox::ScreenBox(uint8_t xPositionChars, const char* charSet,
                     uint8_t charSetCount, uint8_t initialIndex)
    : CharSet{charSet},
      CharSetLength{charSetCount},
      xPositionChars{xPositionChars},
      CharIndex{initialIndex} {}

void ScreenBox::Up() {
  if (CharIndex < CharSetLength - 1) {
    CharIndex += 1;
  } else {
    CharIndex = 0;
  }
  Draw();
}

void ScreenBox::Down() {
  if (CharIndex > 0) {
    CharIndex -= 1;
  } else {
    CharIndex = CharSetLength - 1;
  }
  Draw();
}

void ScreenBox::DrawIndicator() const {
  DrawIndicator(true);
}

void ScreenBox::DrawIndicator(bool on) const {
  AutoTwi t;
  Screen_->GU7000_drawImage(xPositionChars * CharDotWidth, CharDotHeight,
                            CharDotWidth, CharDotHeight,
                            on ? gUnderlineImageData : gBlankImageData);
}

void ScreenBox::Draw() const {
  AutoTwi t;
  Screen_->print(xPositionChars * CharDotWidth, 0, CharSet[CharIndex]);
}

uint8_t ScreenBox::GetCurrentIndex() const {
  return CharIndex;
}

// ================================================================ //

Noritake_VFD_GU7000* ScreenC::Screen_ = nullptr;

ScreenC::ScreenC(const char* title, ThermoSaveData& s,
                 ScreenBoxStorage* boxStorage)
    : SaveData_{s}, Boxes_{boxStorage}, BoxesCount_{3} {
  InitBoxes(boxStorage);
  AutoTwi t;
  Screen_->print(title);
  Screen_->GU7000_setCursor(0, 8);
  for (uint8_t i = 0; i < BoxesCount_; ++i) {
    GetBox(i).Draw();
  }
}

ScreenC::~ScreenC() {
  AutoTwi t;
  Screen_->GU7000_clearScreen();
  ReadBoxes();
}

void ScreenC::InitBoxes(ScreenBoxStorage* s) {
  ::new (s[0].get_address<ScreenBox>()) ScreenBox(0, "CF", 2, 0);
  ::new (s[1].get_address<ScreenBox>()) ScreenBox(1, "A$C#", 4, 0);
  ::new (s[2].get_address<ScreenBox>()) ScreenBox(2, "123", 3, 2);
}

void ScreenC::ReadBoxes() {
  SaveData_.temp_display_unit =
      GetBox(0).GetCurrentIndex() == 0 ? TEMP_UNIT_CELSIUS : TEMP_UNIT_FREEDOM;
}

void ScreenC::OnUpPressed() {
  if (Locked_) {
    CurrentBox().Up();
  } else {
    if (CursorPosition < BoxesCount_ - 1) {
      CursorPosition += 1;
      CurrentBox().DrawIndicator();
    }
  }
}

void ScreenC::OnDownPressed() {
  if (Locked_) {
    CurrentBox().Down();
  } else {
    if (CursorPosition > 0) {
      CursorPosition -= 1;
      CurrentBox().DrawIndicator();
    }
  }
}

void ScreenC::OnSelectPressed() {
  if (!Locked_) {
    Locked_ = true;
    CurrentBox().DrawIndicator();
  } else {
    Locked_ = false;
  }
}

void ScreenC::OnHalfSecondPassed() {
  if (!Locked_) {
    ShowIndicator_ = !ShowIndicator_;
    CurrentBox().DrawIndicator(ShowIndicator_);
  }
}

ScreenBox& ScreenC::CurrentBox() const {
  return GetBox(CursorPosition);
}

ScreenBox& ScreenC::GetBox(uint8_t i) const {
  return Boxes_[i].get_reference<ScreenBox>();
}
