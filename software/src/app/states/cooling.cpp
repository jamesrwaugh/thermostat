#include "cooling.hpp"

#include <driver_rs_wrapper.hpp>

#include "machine.hpp"

Cooling::Cooling(Machine& machine)
    : CoolableParent(machine, State::Type::Cooling) {
  machine_.EnterHeatingOrCooling(HeatModeT::Cooling);
}

Cooling::~Cooling() {
  machine_.ExitHeatingOrCooling();
}
