#include "idle.hpp"

#include <driver_rs_wrapper.hpp>

#include "machine.hpp"

Idle::Idle(Machine& machine) : CoolableParent(machine, State::Type::Idle) {
  DriverDisplayIsIdle();
}

Idle::~Idle() {
  machine_.ResetStateChangeData();
}

State::Type Idle::handle_event(const Event::Base& event) {
  return CoolableParent::handle_event(event);
}