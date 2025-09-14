#include "idle.hpp"

#include <driver_rs_wrapper.hpp>

Idle::Idle(Machine& machine) : machine_(machine) {
  auto v = HeatingModeChangedEvent{.new_mode = HEATING_COMM_IDLE};
  machine_.Comms()(v);
  DriverDisplayIsIdle();
}

Idle::~Idle() {
  machine_.ResetStateChangeData();
}

State::Type::TheType Idle::handle_event(const Event::Base& event) {
  return State::Type::Idle;  // Stay in idle state
}