#include "program_screen.hpp"

#include <Noritake_VFD_GU7000.h>
#include <ThermoSaveData_bp.h>
#include <driver_ds1307.h>
#include <etl/algorithm.h>
#include <etl/placement_new.h>
#include <stdint.h>

#include "state.hpp"

// ================================================================ //

Noritake_VFD_GU7000* ProgramScreenState::Screen_ = nullptr;

ProgramScreenState::ProgramScreenState(const ScreenConfig& s)
    : State::Base(s.stateId), Config_{s} {}

void ProgramScreenState::InitDisplay(bool startOnEndBox) {
  AutoTwi t;
  Screen_->GU7000_clearScreen();
  Screen_->GU7000_setCursor(0, 0);
  Screen_->print(Config_.title);
  for (uint8_t i = 0; i < Config_.boxesCount; ++i) {
    GetBox(i).Draw();
  }
  for (uint8_t i = 0; i < StaticsCount_; ++i) {
    const auto& s = Statics_[i].get_reference<StaticScreenBox>();
    Screen_->print(s.xPosChars * CharDotWidth, 0, s.Character);
  }
  if (startOnEndBox) {
    CursorPosition = Config_.boxesCount - 1;
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
    if (CursorPosition < Config_.boxesCount - 1) {
      CurrentBox().DrawIndicator(false);
      CursorPosition += 1;
      CurrentBox().DrawIndicator();
      HasEditedCurrentBox_ = false;
    } else {
      return Config_.nextState;
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
      return Config_.prevState;
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

static constexpr ScreenConfig kTempScreenConfig{
    .stateId = State::Type::ProgramTemp,
    .title = "Temp Unit",
    .boxesCount = 1,
    .prevState = State::Type::Idle,
    .nextState = State::Type::ProgramDate,
};

constexpr const char* FCCharSet = "FC";
constexpr uint8_t FCCharSetLen = 2;

TempScreen::TempScreen(ThermoSaveData& s, bool startOnEndBox)
    : ProgramScreenState(kTempScreenConfig), S_{s} {
  // Temp Unit C or F
  ::new (GetBoxP(0))
      CharSetScreenBox(15, &s.temp_display_unit, FCCharSet, FCCharSetLen, 1);

  // Init
  InitDisplay(startOnEndBox);
}

// ================================================================ //

static constexpr ScreenConfig kDateScreenConfig{
    .stateId = State::Type::ProgramDate,
    .title = "Date",
    .boxesCount = 4,
    .prevState = State::Type::ProgramTemp,
    .nextState = State::Type::ProgramTime,
};

constexpr const char* DaysOfWeek = "SuMoTuWdThFrSa";
constexpr uint8_t DaysOfWeekSetLen = 14;

DateScreen::DateScreen(ds1307_time_s& s, bool startOnEndBox)
    : ProgramScreenState(kDateScreenConfig), time_{s} {
  // Copy
  year_ = s.year - 2000;

  // Year, Month, Day
  ::new (GetBoxP(0)) TwoDigitScreenBox(5, &s.month, 1, 12);
  ::new (GetBoxP(1)) TwoDigitScreenBox(8, &s.date, 1, 31);
  ::new (GetBoxP(2)) TwoDigitScreenBox(11, &year_, 0, 99);

  // Day of Week
  ::new (GetBoxP(3))
      CharSetScreenBox(14, &s.week, DaysOfWeek, DaysOfWeekSetLen, 2);

  // Separators
  AddStatic(7, '/');
  AddStatic(10, '/');

  // Init
  InitDisplay(startOnEndBox);
}

DateScreen::~DateScreen() {
  time_.year = year_ + 2000;
}

// ================================================================ //

static constexpr ScreenConfig kTimeScreenConfig{
    .stateId = State::Type::ProgramTime,
    .title = "Time",
    .boxesCount = 4,
    .prevState = State::Type::ProgramDate,
    .nextState = State::Type::Idle,
};

constexpr const char* AmPm = "AMPM";
constexpr uint8_t AmPmLen = 4;

TimeScreen::TimeScreen(ds1307_time_s& s, bool startOnEndBox)
    : ProgramScreenState(kTimeScreenConfig), time_{s} {
  // Copy
  am_pm_ = s.am_pm == ds1307_am_pm_t::DS1307_AM ? 0 : 1;

  // Hour, Minute, Second
  ::new (GetBoxP(0)) TwoDigitScreenBox(5, &s.hour, 0, 11);
  ::new (GetBoxP(1)) TwoDigitScreenBox(8, &s.minute, 0, 59);
  ::new (GetBoxP(2)) TwoDigitScreenBox(11, &s.second, 0, 59);

  // AM/PM
  ::new (GetBoxP(3)) CharSetScreenBox(14, &am_pm_, AmPm, AmPmLen, 2);

  // Separators
  AddStatic(7, ':');
  AddStatic(10, ':');

  // Init
  InitDisplay(startOnEndBox);
}

TimeScreen::~TimeScreen() {
  time_.am_pm =
      am_pm_ == 0 ? ds1307_am_pm_t::DS1307_AM : ds1307_am_pm_t::DS1307_PM;
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
