#include <avr/interrupt.h>
#include <stdint.h>

extern "C" {
#include "thermo.h"
}

int main() {
  CHardwareDrivers d;
  d.read_temp_c_function = nullptr;

  ThermoInit(&d);
  uint8_t set = ThermoGetSetPoint();
  ThermoUpButtonPressed();
  ThermoSecondPassed();

  return set;
}
