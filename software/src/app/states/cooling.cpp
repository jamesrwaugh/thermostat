#include "cooling.hpp"

#include <driver_rs_wrapper.hpp>

#include "machine.hpp"

Cooling::Cooling(Machine& machine)
    : CoolableParent(machine, State::Type::Cooling) {
  DriverDisplayIsCooling();
  machine_.EnterHeatingOrCooling(HeatModeT::Cooling);
  machine_.ActivateCoolingRelays(Relay::Compressor, Relay::Heat,
                                 ReverseValveModeT::OnForCooling);
}

Cooling::~Cooling() {
  machine_.ExitHeatingOrCooling();
}

State::Type Cooling::handle_event(const Event::Base& event) {
  return CoolableParent::handle_event(event);
}
