#include <avr/interrupt.h>
#include <etl/hfsm.h>

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
#include "protos/ThermoStateData_bp.h"
#include "state.hpp"
#include "states/cooling.hpp"
#include "states/heating.hpp"
#include "states/idle.hpp"
#include "states/machine.hpp"
#include "states/running_parent.hpp"

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
      machine.receive(Event::HeatModeChanged{Event::HeatModeT::Heating});
      break;
    case Button::TempCold:
      machine.receive(Event::HeatModeChanged{Event::HeatModeT::Cooling});
      break;
    case Button::TempNone:
      machine.receive(Event::HeatModeChanged{Event::HeatModeT::None});
      break;
    case Button::FanAuto:
      machine.receive(Event::FanModeChanged{Event::FanModeT::Auto});
      break;
    case Button::FanOn:
      machine.receive(Event::FanModeChanged{Event::FanModeT::On});
      break;
  }
}

void OnSerialMessage(const char* message, uint16_t messageLen, void*) {
  (void)message;
  (void)messageLen;
}

void TryReadSavedThermoStateData() {
  uint8_t rtcDataBuf[BYTES_LENGTH_THERMO_STATE_DATA];
  DriverReadFlash(0, rtcDataBuf, BYTES_LENGTH_THERMO_STATE_DATA);

  ThermoStateData data;
  DecodeThermoStateData(&data, rtcDataBuf);

  if (data.magic == THERMO_STATE_DATA_MAGIC) {
    machine.SetThermoStateData(data);
  }
}

int main() {
  RunningParent runningParent;
  Idle idleState;
  Heating heatingState;
  Cooling coolingState;

  etl::array<etl::ifsm_state*, State::Type::COUNT> states = {
      &runningParent,
      &idleState,
      &heatingState,
      &coolingState,
  };

  etl::array<etl::ifsm_state*, 2> runningChildren = {
      &heatingState,
      &coolingState,
  };

  runningParent.set_child_states(runningChildren.data(),
                                 runningChildren.size());

  machine.set_states(states.data(), states.size());

  AvrDriverCallbacks callbacks{
      .OnButtonPressed = OnButtonPressed,
      .OnSerialMessage = OnSerialMessage,
  };

  DriverInit(callbacks, nullptr);

  TryReadSavedThermoStateData();

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
