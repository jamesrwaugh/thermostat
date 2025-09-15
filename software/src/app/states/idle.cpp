#include "idle.hpp"

#include <driver_rs_wrapper.hpp>

#include "machine.hpp"

Idle::Idle(Machine& machine) : CoolableParent(machine, State::Type::Idle) {
  auto v = HeatingModeChangedEvent{.new_mode = HEATING_COMM_IDLE};
  machine_.Comms()(v);
  DriverDisplayIsIdle();
}

Idle::~Idle() {
  machine_.ResetStateChangeData();
}

State::Type Idle::handle_event(const Event::Base& event) {
  return CoolableParent::handle_event(event);
}