#include "cooling.hpp"

#include <driver_rs_wrapper.hpp>

#include "machine.hpp"

Cooling::Cooling(Machine& machine)
    : CoolableParent(machine, State::Type::Cooling) {
  EnterHeatingOrCooling(HeatModeT::Cooling);
}

Cooling::~Cooling() {
  ExitHeatingOrCooling();
}
