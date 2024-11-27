#include <avr/interrupt.h>
#include <stdint.h>

extern "C" {
#include "thermo.h"
}

int main() {
  State s;
  ThermoInit(&s);
  uint8_t set = ThermoGetSetPoint(&s);
  ThermoSetSetPoint(&s, 87);
  return set;
}
