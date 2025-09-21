#include "program.hpp"

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
#include "machine.hpp"
#include "protos/ThermoSaveData_bp.h"

enum class SelectionBoxE : uint8_t {
  TempUnitSelect = 0,
  TimeSet = 1,
};

const char* gNumbersCharacterSet = "0123456789";

Program::SelectionBox Program::Boxes[] = {
    [static_cast<uint8_t>(SelectionBoxE::TempUnitSelect)] =
        {
            .XPositionDots = 0,
            .CharacterStride = 1,
            .CharacterSet = "CF",
            .CharacterSetCount = 2,
        },
    [static_cast<uint8_t>(SelectionBoxE::TimeSet)] =
        {
            .XPositionDots = 0,
            .CharacterStride = 1,
            .CharacterSet = "CF",
            .CharacterSetCount = 2,
        },
};

Program::Program(Machine& machine)
    : State::Base(State::Type::Program), machine_(machine) {
  DirtyData_ = machine_.SaveState();
}

Program::~Program() {
  uint8_t buffer[BYTES_LENGTH_THERMO_SAVE_DATA];
  EncodeThermoSaveData(&DirtyData_, buffer);
  DriverWriteFlash(0, buffer, sizeof(buffer));
  machine_.SetThermoSaveData(DirtyData_);
}

State::Type Program::handle_event(const Event::Base& event) {
  if (event.id_ == Event::Type::HalfSecondPassed) {
    LineOn = !LineOn;
  }

  return State::Type::NO_CHANGE;
}

void Program::SelectionBox::Print() {}