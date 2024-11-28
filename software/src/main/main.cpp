#include <avr/interrupt.h>
#include <stdint.h>

extern "C" {
#include "thermo.h"
}

int main() {
  CHardwareDrivers d;
  d.read_temp_c_function = nullptr;

  CState s;
  ThermoInit(&s, &d);
  uint8_t set = ThermoGetSetPoint(&s);
  ThermoSetSetPoint(&s, 87);

  return set;
}
