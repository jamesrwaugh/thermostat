#include "heating.hpp"

#include <driver_rs_wrapper.hpp>

Heating::Heating(Machine& machine) : machine_(machine) {
  DriverDisplayIsHeating();
  machine_.EnterHeatingOrCooling(HeatModeT::Heating);
  machine_.ActivateCoolingRelays(Relay::Heat, Relay::Compressor,
                                 ReverseValveModeT::OnForHeating);
}

Heating::~Heating() {
  machine_.ExitHeatingOrCooling();
}

State::Type::TheType Heating::handle_event(const Event::Base& event) {
  return State::Type::Heating;  // Stay in heating state
}
