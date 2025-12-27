#include "screen.hpp"

#include <Noritake_VFD_GU7000.h>
#include <etl/placement_new.h>
#include <stdint.h>

#include "ThermoSaveData_bp.h"
#include "utils.hpp"

constexpr uint8_t ImageWidth = 6;
constexpr uint8_t ScreenWithInChars = 16;

const uint8_t gArrowImageData[ImageWidth] = {
    // clang-format off
    0b0001'0000, 
    0b0011'0000, 
    0b0111'1100, 
    0b0011'0000, 
    0b0001'0000,
    0b0000'0000,
    // clang-format on
};

const uint8_t gBlankImageData[ImageWidth] = {
    // clang-format off
    0b0000'0000, 
    0b0000'0000, 
    0b0000'0000, 
    0b0000'0000, 
    0b0000'0000,    
    0b0000'0000,
    // clang-format on
};

// ================================================================ //

Noritake_VFD_GU7000* ScreenBox::Screen_ = nullptr;

ScreenBox::ScreenBox(uint8_t groupOrder, uint8_t groupCount,
                     const char* charSet, uint8_t charSetCount,
                     uint8_t initialIndex)
    : CharSet{charSet},
      CharSetLength{charSetCount},
      xPosChars{
          static_cast<uint8_t>((ScreenWithInChars - groupCount) + groupOrder)},
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
  Screen_->GU7000_drawImage(xPositionDots() + 1, CharDotHeight, ImageWidth, 8,
                            on ? gArrowImageData : gBlankImageData);
}

void ScreenBox::Draw() const {
  AutoTwi t;
  // Screen_->GU7000_setCursor(xPosChars, 0);
  Screen_->print(xPositionDots(), 0, CharSet[CharIndex]);
  // Screen_->print(CharSet[CharIndex]);
}

uint8_t ScreenBox::GetCurrentIndex() const {
  return CharIndex;
}

uint8_t ScreenBox::xPositionDots() const {
  return xPosChars * (CharDotWidth);
}

// ================================================================ //

Noritake_VFD_GU7000* ScreenC::Screen_ = nullptr;

ScreenC::ScreenC(const char* title, ThermoSaveData& s,
                 ScreenBoxStorage* boxStorage)
    : SaveData_{s}, Boxes_{boxStorage}, BoxesCount_{3} {
  InitBoxes();
  AutoTwi t;
  Screen_->GU7000_setCursor(0, 0);
  Screen_->print(title);
  for (uint8_t i = 0; i < BoxesCount_; ++i) {
    GetBox(i).Draw();
  }
}

ScreenC::~ScreenC() {
  AutoTwi t;
  Screen_->GU7000_clearScreen();
  ReadBoxes();
}

void ScreenC::InitBoxes() {
  ::new (GetBoxP(0)) ScreenBox(0, BoxesCount_, "CF", 2, 1);
  ::new (GetBoxP(1)) ScreenBox(1, BoxesCount_, "A$C#", 4, 0);
  ::new (GetBoxP(2)) ScreenBox(2, BoxesCount_, "123", 3, 2);
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
      CurrentBox().DrawIndicator(false);
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
      CurrentBox().DrawIndicator(false);
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

inline ScreenBox* ScreenC::GetBoxP(uint8_t i) const {
  return Boxes_[i].get_address<ScreenBox>();
}
