#include "screen.hpp"

#include <Noritake_VFD_GU7000.h>
#include <etl/placement_new.h>
#include <stdint.h>

#include "ThermoSaveData_bp.h"
#include "utils.hpp"

constexpr uint8_t ImageWidth = 7;
constexpr uint8_t ScreenWidthInChars = 16;

const uint8_t gArrowImageData[ImageWidth] = {
    // clang-format off
    0b0000'1000, 
    0b0001'1000, 
    0b0011'1110, 
    0b0001'1000, 
    0b0000'1000,
    0b0000'0000,
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
          static_cast<uint8_t>((ScreenWidthInChars - groupCount) + groupOrder)},
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
  Draw(true);
}

void ScreenBox::Draw(bool on) const {
  AutoTwi t;
  Screen_->print(xPositionDots(), 0, on ? CharSet[CharIndex] : ' ');
}

uint8_t ScreenBox::GetCurrentIndex() const {
  return CharIndex;
}

uint8_t ScreenBox::xPositionDots() const {
  return xPosChars * CharDotWidth;
}

// ================================================================ //

Noritake_VFD_GU7000* ScreenC::Screen_ = nullptr;

ScreenC::ScreenC(const char* title, ThermoSaveData& s,
                 ScreenBoxStorage* boxStorage, uint8_t boxesCount,
                 ScreenT prevScreen, ScreenT nextScreen)
    : SaveData_{s},
      Boxes_{boxStorage},
      BoxesCount_{boxesCount},
      Title{title},
      NextScreen_{nextScreen},
      PrevScreen_{prevScreen} {}

void ScreenC::InitDisplay() {
  AutoTwi t;
  Screen_->GU7000_setCursor(0, 0);
  Screen_->print(Title);
  for (uint8_t i = 0; i < BoxesCount_; ++i) {
    GetBox(i).Draw();
  }
  CurrentBox().DrawIndicator();
}

ScreenC::~ScreenC() {
  AutoTwi t;
  Screen_->GU7000_clearScreen();
}

ScreenT ScreenC::OnUpPressed() {
  if (Locked_) {
    CurrentBox().Up();
    HasEditedCurrentBox_ = true;
  } else {
    if (CursorPosition < BoxesCount_ - 1) {
      CurrentBox().DrawIndicator(false);
      CursorPosition += 1;
      CurrentBox().DrawIndicator();
      HasEditedCurrentBox_ = false;
    } else {
      return NextScreen_;
    }
  }

  return ScreenT::NO_CHANGE;
}

ScreenT ScreenC::OnDownPressed() {
  if (Locked_) {
    CurrentBox().Down();
    HasEditedCurrentBox_ = true;
  } else {
    if (CursorPosition > 0) {
      CurrentBox().DrawIndicator(false);
      CursorPosition -= 1;
      CurrentBox().DrawIndicator();
      HasEditedCurrentBox_ = false;
    } else {
      return PrevScreen_;
    }
  }

  return ScreenT::NO_CHANGE;
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
  ShowIndicator_ = !ShowIndicator_;
  if (!Locked_) {
    CurrentBox().DrawIndicator(ShowIndicator_);
  } else {
    if (!HasEditedCurrentBox_) {
      CurrentBox().Draw(ShowIndicator_);
    }
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

// ================================================================ //

TempScreen::TempScreen(ThermoSaveData& s, ScreenBoxStorage* boxStorage)
    : ScreenC("Units", s, boxStorage, 3, ScreenT::NO_CHANGE, ScreenT::DateSet) {
  ::new (GetBoxP(0))
      ScreenBox(0, BoxesCount_, "CF", 2,
                s.temp_display_unit == TEMP_UNIT_CELSIUS ? 0 : 1);
}

TempScreen::~TempScreen() {
  SaveData_.temp_display_unit =
      GetBox(0).GetCurrentIndex() == 0 ? TEMP_UNIT_CELSIUS : TEMP_UNIT_FREEDOM;
}

// ================================================================ //

constexpr const char* DigitsCharSet = "0123456789";
constexpr uint8_t DigitsCharSetLen = 10;

DateScreen::DateScreen(ThermoSaveData& s, ScreenBoxStorage* boxStorage)
    : ScreenC("Date", s, boxStorage, 8, ScreenT::TempDisplayUnit,
              ScreenT::DateSet) {
  const auto& date = s.date;

  uint8_t yearTens = (date.year / 10);
  uint8_t yearOnes = (date.year % 10);
  uint8_t monthTens = (date.month / 10);
  uint8_t monthOnes = (date.month % 10);
  uint8_t dayTens = (date.day / 10);
  uint8_t dayOnes = (date.day % 10);

  // Year
  ::new (GetBoxP(0))
      ScreenBox(0, BoxesCount_, DigitsCharSet, DigitsCharSetLen, 2);
  ::new (GetBoxP(1))
      ScreenBox(1, BoxesCount_, DigitsCharSet, DigitsCharSetLen, 0);
  ::new (GetBoxP(2))
      ScreenBox(2, BoxesCount_, DigitsCharSet, DigitsCharSetLen, yearTens);
  ::new (GetBoxP(3))
      ScreenBox(3, BoxesCount_, DigitsCharSet, DigitsCharSetLen, yearOnes);

  // Month
  ::new (GetBoxP(4))
      ScreenBox(4, BoxesCount_, DigitsCharSet, DigitsCharSetLen, monthTens);
  ::new (GetBoxP(5))
      ScreenBox(5, BoxesCount_, DigitsCharSet, DigitsCharSetLen, monthOnes);

  // Days
  ::new (GetBoxP(6))
      ScreenBox(6, BoxesCount_, DigitsCharSet, DigitsCharSetLen, dayTens);
  ::new (GetBoxP(7))
      ScreenBox(7, BoxesCount_, DigitsCharSet, DigitsCharSetLen, dayOnes);
}

DateScreen::~DateScreen() {
  //
}

// ================================================================ //

TimeScreen::TimeScreen(ThermoSaveData& s, ScreenBoxStorage* boxStorage)
    : ScreenC("Time", s, boxStorage, 8, ScreenT::DateSet, ScreenT::NO_CHANGE) {
  const auto& time = s.time;

  uint8_t hourTens = (time.hour / 10);
  uint8_t hourOnes = (time.hour % 10);
  uint8_t minuteTens = (time.minute / 10);
  uint8_t minuteOnes = (time.minute % 10);
  uint8_t secondTens = (time.second / 10);
  uint8_t secondOnes = (time.second % 10);

  // Hour
  ::new (GetBoxP(0))
      ScreenBox(0, BoxesCount_, DigitsCharSet, DigitsCharSetLen, hourTens);
  ::new (GetBoxP(1))
      ScreenBox(1, BoxesCount_, DigitsCharSet, DigitsCharSetLen, hourOnes);

  // Minute
  ::new (GetBoxP(2))
      ScreenBox(2, BoxesCount_, DigitsCharSet, DigitsCharSetLen, minuteTens);
  ::new (GetBoxP(3))
      ScreenBox(3, BoxesCount_, DigitsCharSet, DigitsCharSetLen, minuteOnes);

  // Second
  ::new (GetBoxP(4))
      ScreenBox(4, BoxesCount_, DigitsCharSet, DigitsCharSetLen, secondTens);
  ::new (GetBoxP(5))
      ScreenBox(5, BoxesCount_, DigitsCharSet, DigitsCharSetLen, secondOnes);
}

TimeScreen::~TimeScreen() {
  //
}