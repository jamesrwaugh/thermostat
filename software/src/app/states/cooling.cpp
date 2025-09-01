#include "cooling.hpp"

#include <driver_rs_wrapper.hpp>

etl::fsm_state_id_t Cooling::on_enter_state() {
  DriverDisplayIsCooling();

  ThermostatData data;
  DriverGetThermostatType(&data);

  if (data.ReverseValveType == ReverseValveTypeE::EnergizeToCool) {
    DriverRelayOn(Relay::ReversingValve);
  } else {
    DriverRelayOff(Relay::ReversingValve);
  }

  DriverRelayOn(Relay::Compressor);
  DriverRelayOff(Relay::Heat);
  DriverRelayOn(Relay::Fan);

  return No_State_Change;
}

void Cooling::on_exit_state() {
  get_fsm_context().ResetStateChangeData();
}

etl::fsm_state_id_t Cooling::on_event(const Event::SecondPassed&) {
  get_fsm_context().TickChangeCounter();
  return No_State_Change;
}

etl::fsm_state_id_t Cooling::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}
