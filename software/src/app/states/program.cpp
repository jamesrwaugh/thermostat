#include "program.hpp"

#include <Noritake_VFD_GU7000.h>

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
#include "machine.hpp"
#include "protos/ThermoSaveData_bp.h"

enum class SelectionBoxE : uint8_t {
  TempUnitSelect = 0,
};

constexpr const char* gTempSelectCharacterSet = "FC";
constexpr const char* gNumbersCharacterSet = "0123456789";

constexpr Program::SelectionBox Boxes[] = {
    {
        .XPositionDots = 10,
        .CharacterStride = 1,
        .CharacterSet = gTempSelectCharacterSet,
        .CharacterSetCount = 2,
    },
    {
        .XPositionDots = 20,
        .CharacterStride = 1,
        .CharacterSet = gTempSelectCharacterSet,
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

  Boxes[0].Draw(SelectedCharacterIndex);
  Boxes[1].Draw(SelectedCharacterIndex);

  return State::Type::NO_CHANGE;
}

void Program::SelectionBox::Draw(uint8_t characterIndex) const {
  auto& s = DriverGetScreenHandle();
  s.GU7000_setCursor(XPositionDots, 0);
  s.print(CharacterSet[characterIndex * CharacterStride]);
}

const uint8_t gUnderlineImageData[5] = {
    0b00000001, 0b00000001, 0b00000001, 0b00000001, 0b00000001,
};

void Program::EnterTemp(Noritake_VFD_GU7000& s) {
  s.print("TEMP");
  s.GU7000_drawImage(0, 16, 5, 8, gUnderlineImageData);
  s.GU7000_setCursor(0, 1);
  s.print(DirtyData_.temp_display_unit == TEMP_UNIT_FREEDOM ? "F" : "C");
}