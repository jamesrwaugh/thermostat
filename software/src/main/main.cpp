#include "Serial/HardwareSerial.h"
#include <avr/interrupt.h>
#include <avr/io.h>
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

  // Relays
  DDRB |= _BV(DDB2);
  PORTB &= ~_BV(PORTB2);
  DDRC |= _BV(DDC0) | _BV(DDC1) | _BV(DDC2);
  PORTC &= ~_BV(PORTC0);
  PORTC &= ~_BV(PORTC1);
  PORTC &= ~_BV(PORTC2);
}

void setup_interrupts() {
  // For approx-second-timer for temp conversion
  sei();
}

volatile bool RtcSecondPassed = false;
volatile bool RtcHalfSecondPassed = false;

static uint8_t rs_read_temp() {
  //
  return tmp116_read_temp();
}

void setup_timer() { TCCR1A = 2; }

void read_input() {
  // https://www.etlcpp.com/debounce.html
  const int BTN_DEBOUNCE_COUNT = 50;
  const int BTN_HOLD_COUNT = 1000;
  const int BTN_REPEAT_COUNT = 200;
  typedef etl::debounce<BTN_DEBOUNCE_COUNT, BTN_HOLD_COUNT, BTN_REPEAT_COUNT>
      BtnDebounce;

  const int TEMP_DEBOUNCE_COUNT = 50;
  const int TEMP_HOLD_COUNT = 1000;
  const int TEMP_REPEAT_COUNT = 200;
  typedef etl::debounce<TEMP_DEBOUNCE_COUNT, TEMP_HOLD_COUNT, TEMP_REPEAT_COUNT>
      TmpDebounce;

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

int main() {
  Noritake_VFD_GU7000 vfd(13);
  ds3231_handle_t ds;

  setup_interrupts();
  setup_i2c();
  tmp116_init();
  setup_screen(vfd);
  setup_rtc(ds);

  tmp116_read_temp();

  CHardwareDrivers rs;
  memset(&rs, 0, sizeof(rs));
  rs.read_temp_c_function = rs_read_temp;

  ThermoInit(&rs);

  Serial.begin(9600);
  Serial.println("Hello world");

  while (1) {
    if (RtcSecondPassed) {
      RtcSecondPassed = 0;
      ThermoSecondPassed();
    }

    if (RtcHalfSecondPassed) {
      read_input();
    }
  }

  return 0;
}
