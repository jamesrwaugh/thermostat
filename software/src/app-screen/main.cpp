#include <Noritake_VFD_GU7000.h>
#include <stdint.h>

#include <driver_rs_wrapper.hpp>

#include "ThermoSaveData_bp.h"

// 112 x 16
// Temp Display Unit
// Date Set
// Time Set
enum class Screen : uint8_t {
  TempDisplayUnit = 0,
  DateSet = 1,
  TimeSet = 2,
};

const uint8_t gUnderlineImageData[5] = {
    0b00000001, 0b00000001, 0b00000001, 0b00000001, 0b00000001,
};

void EnterTemp(Noritake_VFD_GU7000& s) {
  s.print("TEMP");
  s.GU7000_drawImage(0, 16, 5, 8, gUnderlineImageData);
  s.GU7000_setCursor(0, 1);
  s.print("C");
}

class Program {};

int main() {
  ThermoSaveData data;
  data.magic = THERMO_STATE_DATA_MAGIC;
  data.set_point = 39;
  data.temp_display_unit = TEMP_UNIT_FREEDOM;

  Noritake_VFD_GU7000 s(1);
  EnterTemp(s);

  return 0;
}