#include "heating.hpp"

#include <driver_rs_wrapper.hpp>

etl::fsm_state_id_t Heating::on_enter_state() {
  DriverDisplayIsHeating();

  ThermostatData data;
  DriverGetThermostatType(&data);

  if (data.ReverseValveType == ReverseValveTypeE::EnergizeToHeat) {
    DriverRelayOn(Relay::ReversingValve);
  } else {
    DriverRelayOff(Relay::ReversingValve);
  }

  DriverRelayOff(Relay::Compressor);
  DriverRelayOn(Relay::Heat);
  DriverRelayOn(Relay::Fan);

  return No_State_Change;
}

void Heating::on_exit_state() {
  get_fsm_context().ResetStateChangeData();
}

etl::fsm_state_id_t Heating::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}
