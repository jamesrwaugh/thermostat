#include <avr/interrupt.h>
#include <etl/hfsm.h>

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
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
      machine.receive(Event::UpButtonPressed{});
      break;
    case Button::Down:
      machine.receive(Event::DownButtonPressed{});
      break;
    case Button::Select:
      machine.receive(Event::SelectButtonPressed{});
      break;
    case Button::TempHeat:
      machine.receive(Event::HeatModeChanged{HeatModeT::Heating});
      break;
    case Button::TempCold:
      machine.receive(Event::HeatModeChanged{HeatModeT::Cooling});
      break;
    case Button::TempNone:
      machine.receive(Event::HeatModeChanged{HeatModeT::None});
      break;
    case Button::FanAuto:
      machine.receive(Event::FanModeChanged{FanModeT::Auto});
      break;
    case Button::FanOn:
      machine.receive(Event::FanModeChanged{FanModeT::On});
      break;
    case Button::ReverseValveOnHeat:
      machine.receive(
          Event::ReverseValveModeChanged{ReverseValveModeT::OnForHeating});
      break;
    case Button::ReverseValveOnCool:
      machine.receive(
          Event::ReverseValveModeChanged{ReverseValveModeT::OnForCooling});
      break;
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

  while (true) {
    if (g10MillisecondPassed) {
      g10MillisecondPassed = false;
      lastTenMsCount += 1;
      DriverPollInput();
    }

    if (lastTenMsCount >= 10) {
      lastTenMsCount = 0;
      machine.receive(Event::SecondPassed{});
      uint8_t temp = DriverReadTemp();
      DriverDisplayTemp(temp);
    }

    DriverMcuSleep();
  }

  return 0;
}

ISR(TIMER1_OVF_vect) {
  g10MillisecondPassed = true;
}

void operator delete(void*, unsigned int) {}
