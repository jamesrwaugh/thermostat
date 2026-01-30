#include "heating.hpp"

#include <driver_rs_wrapper.hpp>

#include "images.hpp"
#include "machine.hpp"
#include "states/coolable_parent.hpp"

Heating::Heating(Machine& machine)
    : CoolableParent(machine, State::Type::Heating) {
  EnterHeatingOrCooling(HeatModeT::Heating);
}

State::Type Heating::handle_event(const Event::Base& event) {
  switch (event.id_) {
    case Event::Type::SecondPassed:
      image_state_ = !image_state_;
      DrawImage(DriverGetScreenHandle(), 64, true,
                image_state_ ? gFireOneImageData : gFireTwoImageData);
      break;
    default:
      break;
  }
  return CoolableParent::handle_event(event);
}

Heating::~Heating() {
  ExitHeatingOrCooling();
}
