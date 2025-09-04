#include "cooling.hpp"

#include <driver_rs_wrapper.hpp>

#include "event.hpp"

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

  if (get_fsm_context().ThermoStateData().FanMode() == Event::FanModeT::Auto) {
    DriverRelayOn(Relay::Fan);
  }

  return No_State_Change;
}

void Cooling::on_exit_state() {
  auto& ctx = get_fsm_context();

  if (ctx.ThermoStateData().FanMode() == Event::FanModeT::Auto) {
    DriverRelayOff(Relay::Fan);
  }

  get_fsm_context().ResetStateChangeData();
}

etl::fsm_state_id_t Cooling::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}
