#include "program.hpp"

etl::fsm_state_id_t Program::on_enter_state() {
  return No_State_Change;
}

void Program::on_exit_state() {}

etl::fsm_state_id_t Program::on_event(const Event::SecondPassed&) {
  return No_State_Change;
}

etl::fsm_state_id_t Program::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}