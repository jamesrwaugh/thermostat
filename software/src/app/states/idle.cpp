#include "idle.hpp"

etl::fsm_state_id_t Idle::on_enter_state() {
  return No_State_Change;
}

void Idle::on_exit_state() {
  get_fsm_context().ResetStateChangeData();
}

etl::fsm_state_id_t Idle::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}