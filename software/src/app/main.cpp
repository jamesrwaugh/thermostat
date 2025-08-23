#include <avr/interrupt.h>
#include <etl/hfsm.h>

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
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

void OnSecondPassed(void*) {
  machine.receive(Event::SecondPassed{});
}

void InitInterrupts() {
  // CTC setting top at OCR1A
  TCCR1B |= _BV(WGM12);

  // Prescale clk_io (7372800) / 1024
  TCCR1B |= _BV(CS12) | _BV(CS10);

  // (7372800) / 1024 => 7200 top for 1 ms passed
  OCR1A = 7200;
}

volatile bool gMillisecondPassed = false;
volatile uint16_t gMillisecondsPassed = false;

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
    if (gMillisecondPassed) {
      gMillisecondPassed = false;
      gMillisecondsPassed += 1;
      DriverPollInput();
    }

    if (gMillisecondsPassed >= 1000) {
      gMillisecondsPassed = 0;
      machine.receive(Event::SecondPassed{});
      uint8_t temp = DriverReadTemp();
      DriverDisplayTemp(temp);
    }
  }

  return 0;
}

void operator delete(void*, unsigned int) {}

ISR(TIMER1_OVF_vect) {
  gMillisecondPassed = true;
}