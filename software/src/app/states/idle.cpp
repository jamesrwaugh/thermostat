#include "idle.hpp"

#include <driver_rs_wrapper.hpp>

#include "machine.hpp"

Idle::Idle(Machine& machine) : CoolableParent(machine, State::Type::Idle) {
  if (machine_.SaveState().FanMode() == FanModeT::Auto) {
    machine_.WriteHaActionStateTopicResponse(HaActionKey::Idle);
  } else {
    machine_.WriteHaActionStateTopicResponse(HaActionKey::Fan);
  }
}
