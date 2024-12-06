#include <Noritake_VFD_GU7000.h>
#include <avr/io.h>
#include <twi_master.h>

void initPort() {}

void writePort(const uint8_t data, const uint8_t busyPin) {
  while (PINB & _BV(PINB1)) {
    ;
  }
  tw_write(data);
}

void hardReset() {}

void setup_screen(Noritake_VFD_GU7000 &s) {
  //
  s.GU7000_init();
}