#include "program.hpp"

#include "driver_rs_wrapper.hpp"
#include "protos/ThermoSaveData_bp.h"

etl::fsm_state_id_t Program::on_enter_state() {
  DirtyData_ = get_fsm_context().SaveState();
  return No_State_Change;
}

void Program::on_exit_state() {
  uint8_t buffer[BYTES_LENGTH_THERMO_SAVE_DATA];
  EncodeThermoSaveData(&DirtyData_, buffer);
  DriverWriteFlash(0, buffer, sizeof(buffer));
  get_fsm_context().SetThermoSaveData(DirtyData_);
}

etl::fsm_state_id_t Program::on_event(const Event::SecondPassed&) {
  return No_State_Change;
}

etl::fsm_state_id_t Program::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}