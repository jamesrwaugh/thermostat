#include "heating.hpp"

#include <driver_rs_wrapper.hpp>

#include "machine.hpp"

Heating::Heating(Machine& machine)
    : CoolableParent(machine, State::Type::Heating) {
  DriverDisplayIsHeating();
  machine_.EnterHeatingOrCooling(HeatModeT::Heating);
  machine_.ActivateCoolingRelays(Relay::Heat, Relay::Compressor,
                                 ReverseValveModeT::OnForHeating);
}

Heating::~Heating() {
  machine_.ExitHeatingOrCooling();
}
