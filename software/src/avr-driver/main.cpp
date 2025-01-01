#include "main.hpp"

int main() {
  Serial.begin(9600);
  Serial.println("Hello world");

  uint8_t tenMsCount = 0;

  while (1) {
    if (TimerWakeUp) {
      TimerWakeUp = false;
      tenMsCount += 1;
      read_input();

      if (tenMsCount >= 100) {
        tenMsCount = 0;
        ThermoSecondPassed();
      }
    }

    sleep_cpu();
  }

  return 0;
}

ISR(TIMER1_COMPA_vect) {
  //
  TimerWakeUp = true;
}