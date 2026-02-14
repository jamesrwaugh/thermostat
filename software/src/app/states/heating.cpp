#include "heating.hpp"

#include <driver_rs_wrapper.hpp>

#include "machine.hpp"
#include "states/coolable_parent.hpp"

Heating::Heating(Machine& machine)
    : CoolableParent(machine, State::Type::Heating) {
  EnterHeatingOrCooling(HeatModeT::Heating);
}

Heating::~Heating() {
  ExitHeatingOrCooling();
}
