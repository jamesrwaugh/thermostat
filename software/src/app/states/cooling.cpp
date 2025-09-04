#include "cooling.hpp"

#include <driver_rs_wrapper.hpp>

etl::fsm_state_id_t Cooling::on_enter_state() {
  DriverDisplayIsCooling();
  get_fsm_context().EnterHeatingOrCooling();
  get_fsm_context().ActivateCoolingRelays(Relay::Compressor, Relay::Heat,
                                          ReverseValveModeT::OnForCooling);
  return No_State_Change;
}

void Cooling::on_exit_state() {
  get_fsm_context().ExitHeatingOrCooling();
}

etl::fsm_state_id_t Cooling::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}
