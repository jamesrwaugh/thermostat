#include <Noritake_VFD_GU7000.h>
#include <avr/io.h>
#include <util/delay.h>

#include <driver_rs_wrapper.hpp>

#include "Serial/HardwareSerial.h"
#include "ThermoSaveData_bp.h"
#include "avr/interrupt.h"
#include "avr/iom644p.h"
#include "avr/sfr_defs.h"
#include "screen.hpp"
#include "utils.hpp"

// 112 x 16
// Temp Display Unit
// Date Set
// Time Set
enum class Screen : uint8_t {
  TempDisplayUnit = 0,
  DateSet = 1,
  TimeSet = 2,
};

uint8_t AutoTwi::instanceCount_ = 0;

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

  // LED
  DDRB |= _BV(DDB0);
  PORTB &= ~_BV(PB0);

  // Buttons
  // -> Input pullup
  DDRA &= ~_BV(DDA0);
  DDRA &= ~_BV(DDA1);
  DDRA &= ~_BV(DDA2);
  DDRA &= ~_BV(DDD3);
  PORTA |= _BV(PORTA0) | _BV(PORTA1) | _BV(PORTA2) | _BV(PORTA3);
}

volatile bool g10MillisecondPassed = false;

void SetupInputTimer() {
  // prescaler clk / 1024
  TCCR1B |= _BV(CS12);
  TCCR1B &= ~_BV(CS11);
  TCCR1B |= _BV(CS10);

  // CTC mode on OCR1A, to stop at that value
  // WGM1[3:0] = 0100 = 4
  TCCR1B &= ~_BV(WGM13);
  TCCR1B |= _BV(WGM12);
  TCCR1A &= ~_BV(WGM11);
  TCCR1A &= ~_BV(WGM10);

  // Set OCR1A, approx ~10ms per overflow
  constexpr uint16_t top = 72;
  OCR1A = top;

  // Interrupt on compare A match
  TIMSK1 |= _BV(OCIE1A);
}

int main() {
  setup();
  SetupInputTimer();
  sei();

  ThermoSaveData data;
  data.magic = THERMO_STATE_DATA_MAGIC;
  data.set_point = 39;
  data.temp_display_unit = TEMP_UNIT_FREEDOM;

  ScreenBoxStorage Boxes[5];

  Noritake_VFD_GU7000 gu7k(1);

  ScreenBox::Screen_ = &gu7k;
  ScreenC::Screen_ = &gu7k;

  {
    AutoTwi twi;
    gu7k.GU7000_init();
    gu7k.GU7000_clearScreen();
  }

  ScreenC tempScreen("TEMP", data, Boxes);

  DebounceState UpButton;
  DebounceState DownButton;
  DebounceState SelectButton;
  DebounceState TimeButton;

  bool ledOn = false;
  uint8_t ledCount = 0;

  Serial.begin();

  while (1) {
    uint8_t pina = PINA;

    if (g10MillisecondPassed) {
      g10MillisecondPassed = false;

      if (UpButton.Add(pina & _BV(PINA0))) {
        tempScreen.OnUpPressed();
        Serial.print("Up");
      } else if (DownButton.Add(pina & _BV(PINA1))) {
        tempScreen.OnDownPressed();
        Serial.print("Down");
      } else if (SelectButton.Add(pina & _BV(PINA2))) {
        tempScreen.OnSelectPressed();
        Serial.print("Select");
      } else if (TimeButton.Add(pina & _BV(PINA3))) {
        tempScreen.OnHalfSecondPassed();
        Serial.print("Time");
      }

      ledCount += 1;
      if (ledCount > 100) {
        ledCount = 0;
        ledOn = !ledOn;
        if (ledOn) {
          PORTB |= _BV(PB0);
        } else {
          PORTB &= ~_BV(PB0);
        }
      }
    }
  }

  return 0;
}

ISR(TIMER1_COMPA_vect) {
  g10MillisecondPassed = true;
}