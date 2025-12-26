#include <Noritake_VFD_GU7000.h>
#include <avr/io.h>
#include <stdint.h>

#include <driver_rs_wrapper.hpp>

#include "ThermoSaveData_bp.h"
#include "twi_master.h"
#include "util/delay.h"

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
  s.GU7000_setCursor(0, 8);
  s.print("C");
  s.GU7000_drawImage(1, 8, 5, 8, gUnderlineImageData);
}

class Program {};

struct AutoTwi final {
  static constexpr uint8_t Gu7000SlaveAddr_ = 0x50;

  AutoTwi() {
    tw_master_setup_transmit(Gu7000SlaveAddr_);
  }

  ~AutoTwi() {
    tw_master_end_transmit();
  }
};

void setup() {
  // initialize twi prescaler and bit rate (250k)
  // Set prescaler value of 1
  TWSR &= ~_BV(TWPS0);
  TWSR &= ~_BV(TWPS1);
  TWBR = ((F_CPU / 250000) - 16) / 2;

  // Screen Busy
  // -> Input
  DDRB &= ~_BV(DDB1);
  PORTB &= ~_BV(PORTB1);
}

int main() {
  setup();

  ThermoSaveData data;
  data.magic = THERMO_STATE_DATA_MAGIC;
  data.set_point = 39;
  data.temp_display_unit = TEMP_UNIT_FREEDOM;

  Noritake_VFD_GU7000 s(1);

  {
    AutoTwi twi;
    s.GU7000_init();
  }

  {
    AutoTwi twi;
    EnterTemp(s);
  }

  while (1) {
    AutoTwi twi;
    _delay_ms(500);
    s.GU7000_setCursor(0, 8);
    s.print("C");
    _delay_ms(500);
    s.GU7000_drawImage(0, 7, 5, 8, gUnderlineImageData);
  }

  return 0;
}