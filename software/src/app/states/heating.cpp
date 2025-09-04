#include "heating.hpp"

#include <driver_rs_wrapper.hpp>

etl::fsm_state_id_t Heating::on_enter_state() {
  DriverDisplayIsHeating();
  get_fsm_context().EnterHeatingOrCooling();
  get_fsm_context().ActivateCoolingRelays(Relay::Heat, Relay::Compressor,
                                          ReverseValveModeT::OnForHeating);
  return No_State_Change;
}

void Heating::on_exit_state() {
  get_fsm_context().ExitHeatingOrCooling();
}

etl::fsm_state_id_t Heating::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}
