#include "cooling.hpp"

#include <driver_rs_wrapper.hpp>

Cooling::Cooling(Machine& machine) : machine_(machine) {
  DriverDisplayIsCooling();
  machine_.EnterHeatingOrCooling(HeatModeT::Cooling);
  machine_.ActivateCoolingRelays(Relay::Compressor, Relay::Heat,
                                 ReverseValveModeT::OnForCooling);
}

Cooling::~Cooling() {
  machine_.ExitHeatingOrCooling();
}

State::Type::TheType Cooling::handle_event(const Event::Base& event) {
  return State::Type::Cooling;  // Stay in cooling state
}
