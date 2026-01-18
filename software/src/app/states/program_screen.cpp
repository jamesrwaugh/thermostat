#include "program_screen.hpp"

#include <Noritake_VFD_GU7000.h>
#include <ThermoSaveData_bp.h>
#include <etl/algorithm.h>
#include <etl/placement_new.h>
#include <stdint.h>

#include "state.hpp"

// ================================================================ //

Noritake_VFD_GU7000* ProgramScreenState::Screen_ = nullptr;

ProgramScreenState::ProgramScreenState(State::Type stateId, const char* title,
                                       ThermoSaveData& s, uint8_t boxesCount,
                                       State::Type prevState,
                                       State::Type nextState)
    : State::Base(stateId),
      SaveData_{s},
      BoxesCount_{boxesCount},
      Title{title},
      PrevState_{prevState},
      NextState_{nextState} {}

void ProgramScreenState::InitDisplay(bool startOnEndBox) {
  AutoTwi t;
  Screen_->GU7000_clearScreen();
  Screen_->GU7000_setCursor(0, 0);
  Screen_->print(Title);
  for (uint8_t i = 0; i < BoxesCount_; ++i) {
    GetBox(i).Draw();
  }
  for (uint8_t i = 0; i < StaticsCount_; ++i) {
    const auto& s = Statics_[i].get_reference<StaticScreenBox>();
    Screen_->print(s.xPosChars * CharDotWidth, 0, s.Character);
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
    default:
      break;
  }

  return State::Type::NO_CHANGE;
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

constexpr const char* CorFCharSet = "CF";
constexpr uint8_t CorFLen = 2;

TempScreen::TempScreen(ThermoSaveData& s, bool startOnEndBox)
    : ProgramScreenState(State::Type::ProgramTemp, "Temp Unit", s, 1,
                         State::Type::Idle, State::Type::ProgramDate) {
  // Temp Unit C or F
  auto& tempUnit = s.temp_display_unit;
  ::new (GetBoxP(0)) CharSetScreenBox(15, &tempUnit, CorFCharSet, CorFLen, 1);

  // Init
  InitDisplay(startOnEndBox);
}

// ================================================================ //

constexpr const char* DaysOfWeek = "SuMoTuWdThFrSa";
constexpr uint8_t DaysOfWeekSetLen = 14;

DateScreen::DateScreen(ThermoSaveData& s, bool startOnEndBox)
    : ProgramScreenState(State::Type::ProgramDate, "Date", s, 4,
                         State::Type::ProgramTemp, State::Type::ProgramTime) {
  auto& date = s.date;

  // Year, Month, Day
  ::new (GetBoxP(0)) TwoDigitScreenBox(5, &date.year, 0, 99);
  ::new (GetBoxP(1)) TwoDigitScreenBox(8, &date.month, 1, 12);
  ::new (GetBoxP(2)) TwoDigitScreenBox(11, &date.day, 1, 31);

  // Day of Week
  ::new (GetBoxP(3))
      CharSetScreenBox(14, &date.day_of_week, DaysOfWeek, DaysOfWeekSetLen, 2);

  // Separators
  AddStatic(7, '/');
  AddStatic(10, '/');

  // Init
  InitDisplay(startOnEndBox);
}

// ================================================================ //

constexpr const char* AmPm = "AMPM";
constexpr uint8_t AmPmLen = 4;

TimeScreen::TimeScreen(ThermoSaveData& s, bool startOnEndBox)
    : ProgramScreenState(State::Type::ProgramTime, "Time", s, 4,
                         State::Type::ProgramDate, State::Type::Idle) {
  auto& time = s.time;

  // Hour, Minute, Second
  ::new (GetBoxP(0)) TwoDigitScreenBox(5, &time.hour, 0, 11);
  ::new (GetBoxP(1)) TwoDigitScreenBox(8, &time.minute, 0, 59);
  ::new (GetBoxP(2)) TwoDigitScreenBox(11, &time.second, 0, 59);

  // AM/PM
  ::new (GetBoxP(3)) CharSetScreenBox(14, &time.am_pm, AmPm, AmPmLen, 2);

  // Separators
  AddStatic(7, ':');
  AddStatic(10, ':');

  // Init
  InitDisplay(startOnEndBox);
}

/*

0123456789ABCDEF
TIME 01:23:45 AM
DATE 01/22/25 Su

Auto Temp  1 2 3
           ^

Day Set
Start Mo  End Mo

Start Time
01:23:45 AM

End Time
01:23:45 AM

Temp Setting
Heat     80c

*/
