#pragma once

#include <Noritake_VFD_GU7000.h>

#include <images.hpp>

#include "ThermoSaveData_bp.h"
#include "driver_rs_wrapper.hpp"
#include "event.hpp"
#include "state.hpp"
#include "states/machine.hpp"
#include "states/program_screen.hpp"
#include "states/program_types.hpp"

/*

0123456789ABCDEF
TIME 01:23:45 AM
DATE 01/22/25 Su

Temp Block    u |
1  2  3  4  5  6

Day Set
Start Mo  End Mo

Start Time
01:23:45 AM

End Time
01:23:45 AM

Temp Setting
Heat     80c

*/

class ProgramAutoTimeSelectScreen : State::Base {
 public:
  ProgramAutoTimeSelectScreen(Machine& m)
      : State::Base(State::Type::ProgramAutoTimeSelect), m_{m} {
    m.ResetAutoTimeData();
    AutoTwi t;
    auto& s = DriverGetScreenHandle();
    s.GU7000_clearScreen();
    s.GU7000_setCursor(0, 0);
    s.print("Auto Select  1 2 3");
  }

  State::Type handle_event(const Event::Base& event) override {
    switch (event.id_) {
      case Event::Type::UpButtonPressed:
        Redraw(false);
        if (selection_ == 2) {
          selection_ = 0;
        } else {
          selection_ += 1;
        }
        Redraw(true);
        break;
      case Event::Type::DownButtonPressed:
        Redraw(false);
        if (selection_ == 0) {
          selection_ = 2;
        } else {
          selection_ -= 1;
        }
        Redraw(true);
        break;
      case Event::Type::SelectButtonPressed:
        m_.AutoTimeData().Selection_ = selection_;
        return State::Type::ProgramAutoTimeDates;
        break;
      default:
        break;
    }
    return State::Type::NO_CHANGE;
  }

  void Redraw(bool on) {
    AutoTwi t;
    auto& s = DriverGetScreenHandle();
    s.GU7000_drawImage(20 + (selection_ * CharDotWidth) + 1, CharDotHeight,
                       ImageWidth, 8, on ? gUpArrowImageData : gBlankImageData);
  }

 private:
  Machine& m_;
  uint8_t selection_{0};
};

class ProgramAutoTimeDates : ProgramScreenState {
  ProgramAutoTimeDates(ThermoSaveData& s)
      : ProgramScreenState(State::Type::ProgramAutoTimeDates, "Day Set", s, 1,
                           State::Type::ProgramAutoTimeSelect,
                           State::Type::ProgramAutoTimeStart) {}
}
