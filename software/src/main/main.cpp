#include "Serial/HardwareSerial.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <etl/debounce.h>
#include <rtc_interface.hpp>
#include <screen_interface.hpp>
#include <tmp116.h>

extern "C" {
#include "thermo.h"
}

void setup_i2c() {
  // set no pullups for SDA / SCL
  DDRC &= ~_BV(PC1);
  DDRC &= ~_BV(PC0);
  PORTC &= ~_BV(PC1);
  PORTC &= ~_BV(PC0);

  // initialize twi prescaler and bit rate (250k)
  // Set prescaler value of 1
  TWSR &= ~_BV(TWPS0);
  TWSR &= ~_BV(TWPS1);
  TWBR = ((F_CPU / 50000) - 16) / 2;
}

void setup_pins() {
  // Up button, down button, cooling, heating
  // -> Input pullup
  DDRD &= ~_BV(DDD2);
  DDRD &= ~_BV(DDD3);
  DDRD &= ~_BV(DDD6);
  DDRD &= ~_BV(DDD7);
  PORTD |= _BV(PORTD2) | _BV(PORTD3) | _BV(PORTD6) | _BV(PORTD7);

  // Screen Busy
  // -> Input
  DDRB &= ~_BV(DDB1);
  PORTB &= ~_BV(PORTB1);

  // Relay putput pins, outputs
  DDRB |= _BV(DDB2);
  PORTB &= ~_BV(PORTB2);
  DDRC |= _BV(DDC0) | _BV(DDC1) | _BV(DDC2);
  PORTC &= ~_BV(PORTC0);
  PORTC &= ~_BV(PORTC1);
  PORTC &= ~_BV(PORTC2);
}

static uint8_t rs_read_temp() {
  //
  return tmp116_read_temp();
}

void setup_timer() {
  // prescaler clk / 1024
  TCCR1A |= _BV(CS12);
  TCCR1A &= ~_BV(CS11);
  TCCR1A |= _BV(CS10);

  // CTC mode on OCR1, to stop at that value
  // WGM1[3:0] = 0100 = 4
  TCCR1B &= ~_BV(WGM13);
  TCCR1B |= _BV(WGM12);
  TCCR1A &= ~_BV(WGM11);
  TCCR1A &= ~_BV(WGM10);

  // Set OCR1, approx ~10ms per overflow
  constexpr uint16_t top = 72;
  OCR1A = top;

  // Interrupt on compare A match
  TIMSK1 |= _BV(OCIE1A);
}

// https://www.etlcpp.com/debounce.html
const int BTN_DEBOUNCE_COUNT = 5;
const int BTN_HOLD_COUNT = 50;
const int BTN_REPEAT_COUNT = 200;
typedef etl::debounce<BTN_DEBOUNCE_COUNT, BTN_HOLD_COUNT, BTN_REPEAT_COUNT>
    BtnDebounce;

const int TEMP_DEBOUNCE_COUNT = 200;
const int TEMP_HOLD_COUNT = 1000;
const int TEMP_REPEAT_COUNT = 2000;
typedef etl::debounce<TEMP_DEBOUNCE_COUNT, TEMP_HOLD_COUNT, TEMP_REPEAT_COUNT>
    TmpDebounce;

void read_input() {
  static BtnDebounce upButton;
  static BtnDebounce downButton;
  static BtnDebounce fanOnOff;
  static TmpDebounce tempCoolOn;
  static TmpDebounce tempHeatOn;
  static TmpDebounce tempNone;

  uint8_t pind = PIND;

  if (upButton.add(pind & PIND3) && upButton.is_set()) {
    ThermoUpButtonPressed();
  }

  if (downButton.add(pind & PIND2) && downButton.is_set()) {
    // ThermoDownButtonPressed();
  }

  if (fanOnOff.add(pind & PIND6)) {
    if (fanOnOff.is_set()) {
      // Fan on
    } else {
      // Fan off
    }
  }

  if (tempHeatOn.add(pind & PIND4) && tempHeatOn.is_set()) {
    // Heat on
  }

  if (tempCoolOn.add(pind & PIND5) && tempCoolOn.is_set()) {
    // Cooling on
  }

  if (tempNone.add(!tempCoolOn.is_set() && !tempHeatOn.is_set() &&
                   tempNone.is_held())) {
    // Neither heat nor cooling on
  }
}

volatile bool TimerWakeUp = false;

int main() {
  Noritake_VFD_GU7000 vfd(13);
  ds3231_handle_t ds;

  setup_timer();
  setup_i2c();
  tmp116_init();
  setup_screen(vfd);
  setup_rtc(ds);
  sei();

  tmp116_read_temp();

  CHardwareDrivers rs;
  memset(&rs, 0, sizeof(rs));
  rs.read_temp_c_function = rs_read_temp;

  ThermoInit(&rs);

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
