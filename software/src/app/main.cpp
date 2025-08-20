#include <avr/interrupt.h>
#include <etl/hfsm.h>

#include <driver_rs_wrapper.hpp>

#include "states/cooling.hpp"
#include "states/heating.hpp"
#include "states/idle.hpp"
#include "states/machine.hpp"
#include "states/running_parent.hpp"

Machine machine;

void OnButtonPressed(Button b, void*) {
  switch (b) {
    case Button::Up:
      machine.receive(Event::UpButtonPressed{});
      break;
    case Button::Down:
      machine.receive(Event::DownButtonPressed{});
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

void OnSecondPassed(void*) {
  machine.receive(Event::SecondPassed{});
}

volatile bool gSecondPassed = false;

int main() {
  RunningParent runningParent;
  Idle idleState;
  Heating heatingState;
  Cooling coolingState;

  etl::array<etl::ifsm_state*, 4> states = {
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

  machine.start(true);
  machine.receive(Event::UpButtonPressed{});
  machine.receive(Event::DownButtonPressed{});
  machine.receive(Event::SecondPassed{});

  AvrDriverCallbacks callbacks{
      .OnButtonPressed = OnButtonPressed,
      .OnSerialMessage = OnSerialMessage,
      .OnSecondPassed = OnSecondPassed,
  };

  DriverInit(callbacks, nullptr);

  while (true) {
    DriverMcuSleep();
    uint8_t temp = DriverReadTemp();
    DriverDisplayTemp(temp);
    if (gSecondPassed) {
      gSecondPassed = false;
      machine.receive(Event::SecondPassed{});
    }
  }

  return 0;
}

void operator delete(void*, unsigned int) {}

ISR(TIMER1_OVF_vect) {
  gSecondPassed = true;
}