#include <Noritake_VFD_GU7000.h>
#include <avr/interrupt.h>
#include <driver_ds1307.h>
#include <string.h>

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
#include "state.hpp"
#include "states/machine.hpp"
#include "states/program_screen.hpp"

Machine machine;
volatile bool g10MillisecondPassed = false;

void OnButtonPressed(Button b) {
  switch (b) {
    case Button::Up:
      machine.receive(Event::UpButtonPressed());
      break;
    case Button::Down:
      machine.receive(Event::DownButtonPressed());
      break;
    case Button::Select:
      machine.receive(Event::SelectButtonPressed());
      break;
    case Button::Fan:
      machine.receive(Event::FanButtonPressed());
      break;
    case Button::Heat:
      machine.receive(Event::HeatButtonPressed());
      break;
    case Button::ReverseValveOnHeat:
      machine.receive(
          Event::ReverseValveModeChanged(ReverseValveModeT::OnForHeating));
      break;
    case Button::ReverseValveOnCool:
      machine.receive(
          Event::ReverseValveModeChanged(ReverseValveModeT::OnForCooling));
      break;
  }
}

void PrintStateChange(const char* message) {
  const char* n = "New --> ";
  DriverWriteSerialPortRaw((uint8_t*)n, strlen(n));
  DriverWriteSerialPortRaw((uint8_t*)message, strlen(message));
  DriverWriteSerialPortRaw((uint8_t*)"\r\n", 2);
}

int main() {
  DriverInit();

  ds1307_time_s time = {
      .year = 2025,
      .month = 9,
      .week = 2,
      .date = 16,
      .hour = 6,
      .minute = 37,
      .second = 23,
      .am_pm = ds1307_am_pm_t::DS1307_PM,
  };

  DriverSetTime(time);

  machine.start();

  DriverGetTime(time);

  uint8_t lastHalfSecondCount = 0;
  uint8_t lastSecondCount = 0;

  // Invalid state to force initial update
  State::Type lastState = State::Type::NO_CHANGE;

  ProgramScreenState::Screen_ = &DriverGetScreenHandle();
  ScreenBox::Screen_ = &DriverGetScreenHandle();

  while (true) {
    auto state = machine.get_state_id();

    if (lastState != state) {
      lastState = state;
      // OnStateChange(state);
    }

    if (g10MillisecondPassed) {
      auto button = DriverReadButton();
      if (button != -1) {
        OnButtonPressed(static_cast<Button>(button));
      }
      g10MillisecondPassed = false;
      lastHalfSecondCount += 1;
      lastSecondCount += 1;
    }

    if (lastHalfSecondCount >= 50) {
      lastHalfSecondCount = 0;
      machine.receive(Event::HalfSecondPassed());
    }

    if (lastSecondCount >= 100) {
      lastSecondCount = 0;
      machine.receive(Event::SecondPassed());
    }

    DriverMcuSleep();
  }

  return 0;
}

ISR(TIMER1_COMPA_vect) {
  g10MillisecondPassed = true;
}

void operator delete(void*, unsigned int) {}
