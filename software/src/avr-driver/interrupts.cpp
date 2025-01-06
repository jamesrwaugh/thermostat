#include <avr/interrupt.h>

#include "driver.hpp"

ISR(TIMER1_OVF_vect) {
  gDriver->ReadInput();
}