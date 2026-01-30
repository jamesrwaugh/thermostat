#include "idle.hpp"

#include <driver_rs_wrapper.hpp>

#include "machine.hpp"

Idle::Idle(Machine& machine)
    : CoolableParent(machine, State::Type::Idle, nullptr, nullptr) {
  DriverDisplayIsIdle();
  machine.WriteHaActionStateTopicResponse(HaActionKey::Idle);
}
