#include <avr/interrupt.h>
#include <etl/hfsm.h>

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
#include "protos/ThermoCommEvent_bp.h"
#include "protos/ThermoSaveData_bp.h"
#include "state.hpp"
#include "states/coolable_parent.hpp"
#include "states/cooling.hpp"
#include "states/heating.hpp"
#include "states/idle.hpp"
#include "states/machine.hpp"
#include "states/program.hpp"

Machine machine;
volatile bool g10MillisecondPassed = false;

void OnButtonPressed(Button b, void*) {
  switch (b) {
    case Button::Up:
      DriverWriteSerialPortS("Up");
      machine.receive(Event::UpButtonPressed{});
      break;
    case Button::Down:
      DriverWriteSerialPortS("DN");
      machine.receive(Event::DownButtonPressed{});
      break;
    case Button::Select:
      DriverWriteSerialPortS("SEL");
      machine.receive(Event::SelectButtonPressed{});
      break;
    case Button::TempHeat:
      DriverWriteSerialPortS("TH");
      machine.receive(Event::HeatModeChanged{HeatModeT::Heating});
      break;
    case Button::TempCold:
      DriverWriteSerialPortS("TC");
      machine.receive(Event::HeatModeChanged{HeatModeT::Cooling});
      break;
    case Button::TempNone:
      DriverWriteSerialPortS("TN");
      machine.receive(Event::HeatModeChanged{HeatModeT::None});
      break;
    case Button::FanAuto:
      DriverWriteSerialPortS("FA");
      machine.receive(Event::FanModeChanged{FanModeT::Auto});
      break;
    case Button::FanOn:
      DriverWriteSerialPortS("FO");
      machine.receive(Event::FanModeChanged{FanModeT::On});
      break;
    case Button::ReverseValveOnHeat:
      DriverWriteSerialPortS("RVOH");
      machine.receive(
          Event::ReverseValveModeChanged{ReverseValveModeT::OnForHeating});
      break;
    case Button::ReverseValveOnCool:
      DriverWriteSerialPortS("RVOCO");
      machine.receive(
          Event::ReverseValveModeChanged{ReverseValveModeT::OnForCooling});
      break;
  }
}

void OnSerialMessage(const char* message, uint16_t messageLen, void*) {
  (void)message;
  (void)messageLen;
}

void ReadThermostatStaticState() {
  uint8_t rtcDataBuf[BYTES_LENGTH_THERMO_SAVE_DATA];
  DriverReadFlash(0, rtcDataBuf, sizeof(rtcDataBuf));

  ThermoSaveData data;
  DecodeThermoSaveData(&data, rtcDataBuf);

  if (data.magic == THERMO_STATE_DATA_MAGIC) {
    machine.SetThermoSaveData(data);
  }

  ThermoButtonState buttons;
  DriverGetButtonStateNow(&buttons);
  machine.SetThermoButtonState(buttons);
}

void PrintStateChange(const char* message) {
  DriverWriteSerialPortS("New State: ");
  DriverWriteSerialPortS(message);
}

void OnStateChange(State::Type::TheType state) {
  switch (state) {
    case State::Type::CoolableParent:
      PrintStateChange("CP");
      break;
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
  CoolableParent coolableParent;
  Idle idleState;
  Heating heatingState;
  Cooling coolingState;
  Program programmingState;

  etl::array<etl::ifsm_state*, State::Type::COUNT> states = {
      &coolableParent, &idleState,        &heatingState,
      &coolingState,   &programmingState,
  };

  etl::array<etl::ifsm_state*, 3> coolableChildren = {
      &idleState,
      &heatingState,
      &coolingState,
  };

  coolableParent.set_child_states(coolableChildren.data(),
                                  coolableChildren.size());

  machine.set_states(states.data(), states.size());

  AvrDriverCallbacks callbacks{
      .OnButtonPressed = OnButtonPressed,
      .OnSerialMessage = OnSerialMessage,
  };

  DriverInit(callbacks, nullptr);

  ReadThermostatStaticState();

  machine.start(true);

  uint8_t lastTenMsCount = 0;

  auto v = SetPointChangedEvent{.new_set_point_f = 2};
  machine.Comms()(v);

  const char* message = "Hello, world!";
  DriverWriteSerialPortS(message);

  etl::fsm_state_id_t lastState = etl::ifsm_state::No_State_Change;

  while (true) {
    auto state = machine.get_state_id();

    if (lastState != state) {
      lastState = state;
      OnStateChange(static_cast<State::Type::TheType>(state));
    }

    if (g10MillisecondPassed) {
      DriverPollInput();
      g10MillisecondPassed = false;
      lastTenMsCount += 1;
    }

    if (lastTenMsCount >= 10) {
      lastTenMsCount = 0;
      machine.receive(Event::SecondPassed{});
    }

    DriverMcuSleep();
  }

  return 0;
}

ISR(TIMER1_COMPA_vect) {
  g10MillisecondPassed = true;
}

void operator delete(void*, unsigned int) {}
