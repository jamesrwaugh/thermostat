#include <avr/interrupt.h>
#include <avr/io.h>
#include <rtc_interface.hpp>
#include <screen_interface.hpp>
#include <stdint.h>
#include <tmp116.h>
#include <twi_master.h>
#include <util/delay.h>

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

  // RTC INT
  // -> Input (no pullup, there is an external one)
  DDRC &= ~_BV(DDC3);
  PORTC &= ~_BV(PORTC3);

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
  // 11: rtc int
  PCMSK1 |= _BV(PCINT11);

  // 23/22: fan
  // 21/20: cooling
  // 19: up, 18: down,
  PCMSK2 |= _BV(PCINT23) | _BV(PCINT22) | _BV(PCINT21) | _BV(PCINT20) |
            _BV(PCINT19) | _BV(PCINT18);

  PCICR |= _BV(PCIE1) | _BV(PCIE2);

  sei();
}

enum class NewHeatingModeE : uint8_t {
  None = 0,
  Cooling,
  Heating,
};

enum class NewFanStateE : uint8_t {
  None = 0,
  Off,
  On,
};

// Remember to read initial conditions of these
volatile bool UpButtonPressedFlag = false;
volatile bool DownButtonPressedFlag = false;
volatile bool RtcSecondPassed = false;
volatile NewHeatingModeE NewHeatingModeFlag = NewHeatingModeE::None;
volatile NewFanStateE FanStateChangedFlag = NewFanStateE::None;

int main() {
  Noritake_VFD_GU7000 vfd(13);
  ds3231_handle_t ds;

  setup_interrupts();
  setup_i2c();
  tmp116_init();
  setup_screen(vfd);
  setup_rtc(ds);

  tmp116_read_temp();

  CHardwareDrivers d;
  d.read_temp_c_function = nullptr;

  ThermoInit(&d);
  uint8_t set = ThermoGetSetPoint();
  ThermoUpButtonPressed();
  ThermoSecondPassed();

  return set;
}

// PD2 PCINT18: Down
// PD3 PCINT19: UP
volatile uint8_t gLastPinD = 0;
volatile uint8_t gLastPinC = 0;

inline bool FallingEdge(uint8_t oldPin, uint8_t newPinReg, uint8_t pinIndex) {
  return ((oldPin & _BV(pinIndex)) != 0) && ((newPinReg & _BV(pinIndex)) == 0);
}

inline bool RisingEdge(uint8_t oldPin, uint8_t newPin, uint8_t pin) {
  return ((oldPin & _BV(pin)) == 0) && ((newPin & _BV(pin)) != 0);
}

ISR(PCINT1_vect) {
  uint8_t pin = PINC;

  if (FallingEdge(gLastPinC, pin, PINC3)) {
    RtcSecondPassed = true;
  }

  gLastPinC = pin;
}

ISR(PCINT2_vect) {
  uint8_t pin = PIND;

  if (FallingEdge(gLastPinD, pin, PIND2)) {
    DownButtonPressedFlag = true;
  }

  if (FallingEdge(gLastPinD, pin, PIND3)) {
    UpButtonPressedFlag = true;
  }

  if (FallingEdge(gLastPinD, pin, PIND4)) {
    NewHeatingModeFlag = NewHeatingModeE::Cooling;
  }

  if (FallingEdge(gLastPinD, pin, PIND5)) {
    NewHeatingModeFlag = NewHeatingModeE::Heating;
  }

  if (FallingEdge(gLastPinD, pin, PIND6)) {
    FanStateChangedFlag = NewFanStateE::Off;
  }

  if (FallingEdge(gLastPinD, pin, PIND7)) {
    FanStateChangedFlag = NewFanStateE::On;
  }

  gLastPinD = pin;
}