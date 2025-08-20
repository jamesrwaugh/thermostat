#include "idle.hpp"

etl::fsm_state_id_t Idle::on_enter_state() {
  return No_State_Change;
}

etl::fsm_state_id_t Idle::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}
