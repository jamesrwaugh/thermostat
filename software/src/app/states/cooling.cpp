#include "cooling.hpp"

#include <driver_rs_wrapper.hpp>

etl::fsm_state_id_t Cooling::on_enter_state() {
  DriverDisplayIsCooling();

  DriverRelayOn(Relay::Compressor);
  DriverRelayOff(Relay::Heat);
  DriverRelayOff(Relay::ReversingValve);
  DriverRelayOn(Relay::Fan);

  return No_State_Change;
}

etl::fsm_state_id_t Cooling::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}
