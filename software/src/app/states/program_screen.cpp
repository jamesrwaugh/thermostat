#include "program_screen.hpp"

#include <Noritake_VFD_GU7000.h>
#include <ThermoSaveData_bp.h>
#include <etl/placement_new.h>
#include <stdint.h>

#include "state.hpp"

constexpr uint8_t ImageWidth = 7;

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

ScreenBox::ScreenBox(uint8_t xPosChars, const char* charSet,
                     uint8_t charSetCount, uint8_t initialIndex, uint8_t stride)
    : CharSet{charSet},
      CharSetLength{charSetCount},
      xPosChars{xPosChars},
      CharIndex{initialIndex},
      Stride{stride} {}

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
  for (uint8_t i = 0; i < Stride; ++i) {
    Screen_->print(xPositionDots() + i * CharDotWidth, 0,
                   on ? CharSet[CharIndex + i] : ' ');
  }
}

uint8_t ScreenBox::GetCurrentIndex() const {
  return CharIndex;
}

uint8_t ScreenBox::xPositionDots() const {
  return xPosChars * CharDotWidth;
}

// ================================================================ //

StaticScreenBox::StaticScreenBox(uint8_t xPosChars, char character)
    : xPosChars{xPosChars}, Character{character} {}

// ================================================================ //

Noritake_VFD_GU7000* ProgramScreenState::Screen_ = nullptr;

ProgramScreenState::ProgramScreenState(const char* title, ThermoSaveData& s,
                                       uint8_t boxesCount,
                                       State::Type prevState,
                                       State::Type nextState)
    : State::Base(State::Type::ProgramTemp),
      SaveData_{s},
      BoxesCount_{boxesCount},
      Title{title},
      PrevState_{prevState},
      NextState_{nextState} {}

void ProgramScreenState::InitDisplay() {
  InitDisplay(false);
}

void ProgramScreenState::InitDisplay(bool startOnEndBox) {
  AutoTwi t;
  Screen_->GU7000_setCursor(0, 0);
  Screen_->print(Title);
  for (uint8_t i = 0; i < BoxesCount_; ++i) {
    GetBox(i).Draw();
  }
  if (startOnEndBox) {
    CursorPosition = BoxesCount_ - 1;
  }
  CurrentBox().DrawIndicator();
}

State::Type ProgramScreenState::handle_event(const Event::Base& event) {
  switch (event.id_) {
    case Event::Type::UpButtonPressed:
      return OnUpPressed();
    case Event::Type::DownButtonPressed:
      return OnDownPressed();
    case Event::Type::SelectButtonPressed:
      OnSelectPressed();
      break;
    case Event::Type::HalfSecondPassed:
      OnHalfSecondPassed();
      break;
  }

  return State::Type::NO_CHANGE;
}

ProgramScreenState::~ProgramScreenState() {
  AutoTwi t;
  Screen_->GU7000_clearScreen();
}

State::Type ProgramScreenState::OnUpPressed() {
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
      return NextState_;
    }
  }

  return State::Type::NO_CHANGE;
}

State::Type ProgramScreenState::OnDownPressed() {
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
      return PrevState_;
    }
  }

  return State::Type::NO_CHANGE;
}

void ProgramScreenState::OnSelectPressed() {
  if (!Locked_) {
    Locked_ = true;
    CurrentBox().DrawIndicator();
  } else {
    Locked_ = false;
  }
}

void ProgramScreenState::OnHalfSecondPassed() {
  ShowIndicator_ = !ShowIndicator_;
  if (!Locked_) {
    CurrentBox().DrawIndicator(ShowIndicator_);
  } else {
    if (!HasEditedCurrentBox_) {
      CurrentBox().Draw(ShowIndicator_);
    }
  }
}

const ScreenBox& ProgramScreenState::CurrentBox() const {
  return GetBox(CursorPosition);
}

ScreenBox& ProgramScreenState::CurrentBox() {
  return GetBox(CursorPosition);
}

const ScreenBox& ProgramScreenState::GetBox(uint8_t i) const {
  return Boxes_[i].get_reference<ScreenBox>();
}

ScreenBox& ProgramScreenState::GetBox(uint8_t i) {
  return Boxes_[i].get_reference<ScreenBox>();
}

inline ScreenBox* ProgramScreenState::GetBoxP(uint8_t i) {
  return Boxes_[i].get_address<ScreenBox>();
}

uint8_t ProgramScreenState::GetBoxIndex(uint8_t i) const {
  return GetBox(i).GetCurrentIndex();
}

void ProgramScreenState::AddStatic(uint8_t xPosChars, char character) {
  ::new (&Statics_[StaticsCount_]) StaticScreenBox(xPosChars, character);
  StaticsCount_ += 1;
}

// ================================================================ //

TempScreen::TempScreen(ThermoSaveData& s)
    : ProgramScreenState("Units", s, 1, State::Type::Idle,
                         State::Type::ProgramDate) {
  ::new (GetBoxP(0)) ScreenBox(
      15, "CF", 2, SaveData_.temp_display_unit == TEMP_UNIT_CELSIUS ? 0 : 1, 1);
}

TempScreen::~TempScreen() {
  SaveData_.temp_display_unit =
      GetBoxIndex(0) == 0 ? TEMP_UNIT_CELSIUS : TEMP_UNIT_FREEDOM;
}

// ================================================================ //

constexpr const char* Digits = "0123456789";
constexpr uint8_t DigitsSetLen = 10;

constexpr const char* DaysOfWeek = "SuMoTuWdThFrSa";
constexpr uint8_t DaysOfWeekSetLen = 12;

// Date 01/22/25 Su

DateScreen::DateScreen(ThermoSaveData& s)
    : ProgramScreenState("Date", s, 7, State::Type::ProgramTime,
                         State::Type::Idle) {
  const auto& date = s.date;

  uint8_t yearTens = (date.year / 10);
  uint8_t yearOnes = (date.year % 10);
  uint8_t monthTens = (date.month / 10);
  uint8_t monthOnes = (date.month % 10);
  uint8_t dayTens = (date.day / 10);
  uint8_t dayOnes = (date.day % 10);
  uint8_t dayOfWeek = (date.day_of_week);

  // Year Tens and Ones
  ::new (GetBoxP(0)) ScreenBox(5, Digits, DigitsSetLen, yearTens, 1);
  ::new (GetBoxP(1)) ScreenBox(6, Digits, DigitsSetLen, yearOnes, 1);

  // Month
  ::new (GetBoxP(2)) ScreenBox(8, Digits, DigitsSetLen, monthTens, 1);
  ::new (GetBoxP(3)) ScreenBox(9, Digits, DigitsSetLen, monthOnes, 1);

  // Days
  ::new (GetBoxP(4)) ScreenBox(11, Digits, DigitsSetLen, dayTens, 1);
  ::new (GetBoxP(5)) ScreenBox(12, Digits, DigitsSetLen, dayOnes, 1);

  // Days of Week
  ::new (GetBoxP(6)) ScreenBox(14, DaysOfWeek, DaysOfWeekSetLen, dayOfWeek, 2);

  // Separators
  AddStatic(10, '-');
  AddStatic(13, '-');
}

DateScreen::~DateScreen() {
  SaveData_.date.year = GetBoxIndex(0) * 10 + GetBoxIndex(1);
  SaveData_.date.month = GetBoxIndex(2) * 10 + GetBoxIndex(3);
  SaveData_.date.day = GetBoxIndex(4) * 10 + GetBoxIndex(5);
  SaveData_.date.day_of_week = GetBoxIndex(6);
}

// ================================================================ //

constexpr const char* AmPm = "AP";
constexpr uint8_t AmPmLen = 2;

TimeScreen::TimeScreen(ThermoSaveData& s)
    : ProgramScreenState("Time", s, 7, State::Type::ProgramDate,
                         State::Type::Idle) {
  const auto& time = s.time;

  uint8_t hourTens = (time.hour / 10);
  uint8_t hourOnes = (time.hour % 10);
  uint8_t minuteTens = (time.minute / 10);
  uint8_t minuteOnes = (time.minute % 10);
  uint8_t secondTens = (time.second / 10);
  uint8_t secondOnes = (time.second % 10);
  uint8_t amPmIdx = (time.am_pm == TIME_AM ? 0 : 1);

  // Hour
  ::new (GetBoxP(0)) ScreenBox(6, Digits, DigitsSetLen, hourTens, 1);
  ::new (GetBoxP(1)) ScreenBox(7, Digits, DigitsSetLen, hourOnes, 1);

  // Minute
  ::new (GetBoxP(2)) ScreenBox(9, Digits, DigitsSetLen, minuteTens, 1);
  ::new (GetBoxP(3)) ScreenBox(10, Digits, DigitsSetLen, minuteOnes, 1);

  // Second
  ::new (GetBoxP(4)) ScreenBox(12, Digits, DigitsSetLen, secondTens, 1);
  ::new (GetBoxP(5)) ScreenBox(13, Digits, DigitsSetLen, secondOnes, 1);

  // AM/PM
  ::new (GetBoxP(6)) ScreenBox(15, AmPm, AmPmLen, amPmIdx, 1);

  // Separators
  AddStatic(8, ':');
  AddStatic(11, ':');
}

TimeScreen::~TimeScreen() {
  SaveData_.time.hour = GetBoxIndex(0) * 10 + GetBoxIndex(1);
  SaveData_.time.minute = GetBoxIndex(2) * 10 + GetBoxIndex(3);
  SaveData_.time.second = GetBoxIndex(4) * 10 + GetBoxIndex(5);
  SaveData_.time.am_pm = GetBoxIndex(6) == 0 ? TIME_AM : TIME_PM;
}