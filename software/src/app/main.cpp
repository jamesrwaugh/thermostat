#include <avr/interrupt.h>
#include <driver_ds1307.h>
#include <string.h>

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
#include "state.hpp"
#include "states/machine.hpp"

Machine machine;
volatile bool g10MillisecondPassed = false;

void OnButtonPressed(Button b) {
  switch (b) {
    case Button::Up:
      DriverWriteSerialPortS("Up");
      machine.receive(Event::UpButtonPressed());
      break;
    case Button::Down:
      DriverWriteSerialPortS("DN");
      machine.receive(Event::DownButtonPressed());
      break;
    case Button::Select:
      DriverWriteSerialPortS("SEL");
      machine.receive(Event::SelectButtonPressed());
      break;
    case Button::TempHeat:
      DriverWriteSerialPortS("TH");
      machine.receive(Event::HeatModeChanged(HeatModeT::Heating));
      break;
    case Button::TempCold:
      DriverWriteSerialPortS("TC");
      machine.receive(Event::HeatModeChanged(HeatModeT::Cooling));
      break;
    case Button::TempNone:
      DriverWriteSerialPortS("TN");
      machine.receive(Event::HeatModeChanged(HeatModeT::None));
      break;
    case Button::FanAuto:
      DriverWriteSerialPortS("FA");
      machine.receive(Event::FanModeChanged(FanModeT::Auto));
      break;
    case Button::FanOn:
      DriverWriteSerialPortS("FO");
      machine.receive(Event::FanModeChanged(FanModeT::On));
      break;
    case Button::ReverseValveOnHeat:
      DriverWriteSerialPortS("RVOH");
      machine.receive(
          Event::ReverseValveModeChanged(ReverseValveModeT::OnForHeating));
      break;
    case Button::ReverseValveOnCool:
      DriverWriteSerialPortS("RVOCO");
      machine.receive(
          Event::ReverseValveModeChanged(ReverseValveModeT::OnForCooling));
      break;
  }
}

void OnSerialMessage(const char* message, uint16_t messageLen) {
  (void)message;
  (void)messageLen;
}

void PrintStateChange(const char* message) {
  const char* n = "New State --> ";
  DriverWriteSerialPortRaw((uint8_t*)n, strlen(n));
  DriverWriteSerialPortRaw((uint8_t*)message, strlen(message));
  DriverWriteSerialPortRaw((uint8_t*)"\r\n", 2);
}

void OnStateChange(State::Type state) {
  switch (state) {
    case State::Type::Idle:
      PrintStateChange("ID");
      break;
    case State::Type::Heating:
      PrintStateChange("H");
      break;
    case State::Type::Cooling:
      PrintStateChange("C");
      break;
    case State::Type::Program:
      PrintStateChange("P");
      break;
    case State::Type::COUNT:
      PrintStateChange("?");
      break;
    default:
      PrintStateChange("?");
      break;
  }
}

int main() {
  AvrDriverCallbacks callbacks{
      .OnButtonPressed = OnButtonPressed,
      .OnSerialMessage = OnSerialMessage,
  };

  DriverInit(callbacks);

  ds1307_time_s time = {
      .year = 2025,
      .month = 9,
      .week = 2,
      .date = 16,
      .hour = 6,
      .minute = 37,
      .second = 23,
      .format = ds1307_format_t::DS1307_FORMAT_12H,
      .am_pm = ds1307_am_pm_t::DS1307_PM,
  };

  DriverSetTime(time);

  machine.start();

  DriverGetTime(time);

  uint8_t lastTenMsCount = 0;

  DriverWriteSerialPortS("Hello, world!");

  // Invalid state to force initial update
  State::Type lastState = State::Type::COUNT;

  while (true) {
    auto state = machine.get_state_id();

    if (lastState != state) {
      lastState = state;
      OnStateChange(state);
    }

    if (g10MillisecondPassed) {
      DriverPollInput();
      g10MillisecondPassed = false;
      lastTenMsCount += 1;
    }

    if (lastTenMsCount >= 10) {
      lastTenMsCount = 0;
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
