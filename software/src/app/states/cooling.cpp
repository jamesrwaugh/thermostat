#include "cooling.hpp"

#include <driver_rs_wrapper.hpp>

#include "images.hpp"
#include "machine.hpp"

Cooling::Cooling(Machine& machine)
    : CoolableParent(machine, State::Type::Cooling) {
  EnterHeatingOrCooling(HeatModeT::Cooling);
}

State::Type Cooling::handle_event(const Event::Base& event) {
  switch (event.id_) {
    case Event::Type::SecondPassed:
      image_state_ = !image_state_;
      DrawImage(DriverGetScreenHandle(), 64, true,
                image_state_ ? gSnowflakeOneImageData : gSnowflakeTwoImageData);
      break;
    default:
      break;
  }
  return CoolableParent::handle_event(event);
}

Cooling::~Cooling() {
  ExitHeatingOrCooling();
}
