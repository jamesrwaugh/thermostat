#include "heating.hpp"

etl::fsm_state_id_t Heating::on_enter_state() {
  return No_State_Change;
}

etl::fsm_state_id_t Heating::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}
