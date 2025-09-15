#include "program.hpp"

#include <driver_rs_wrapper.hpp>

#include "machine.hpp"
#include "protos/ThermoSaveData_bp.h"

Program::Program(Machine& machine)
    : State::Base(State::Type::Program), machine_(machine) {
  DirtyData_ = machine_.SaveState();
}

Program::~Program() {
  uint8_t buffer[BYTES_LENGTH_THERMO_SAVE_DATA];
  EncodeThermoSaveData(&DirtyData_, buffer);
  DriverWriteFlash(0, buffer, sizeof(buffer));
  machine_.SetThermoSaveData(DirtyData_);
  machine_.Comms()(DirtyData_);
}

State::Type Program::handle_event(const Event::Base& event) {
  return State::Type::Program;  // Stay in program state
}