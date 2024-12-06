#include <Noritake_VFD_GU7000.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <driver_ds3231.h>
#include <etl/optional.h>
#include <stdint.h>
#include <tmp116.h>
#include <twi_master.h>
#include <util/delay.h>

extern "C" {
#include "thermo.h"
}

uint8_t dummy() {
  // We init IIC elsewhere, just pass this to the DS3231 to do nothing.
  return 0;
}

uint8_t iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
  uint8_t err = 0;
  err |= tw_master_transmit_one(addr, reg, true);
  err |= tw_master_transmit(addr, buf, len, false);
  return !(err == SUCCESS);
}

uint8_t iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
  uint8_t err = 0;
  err |= tw_master_transmit_one(addr, reg, true);
  err |= tw_master_receive(addr, buf, len);
  return !(err == SUCCESS);
}

void debug_print(const char *const fmt, ...) {}

void receive_callback(uint8_t type) {}

void delay_ms(uint32_t ms) {}

void initPort() {}

void writePort(const uint8_t data, const uint8_t busyPin) {
  while (PINB & _BV(PINB1)) {
    ;
  }
  tw_write(data);
}

void hardReset() {}

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
}

void setup_interrupts() {
  // 11: rtc int
  PCMSK1 |= _BV(PCINT11);

  // 19: up, 18: down,
  PCMSK2 |= _BV(PCINT19) | _BV(PCINT18);

  // cooling on / mid / off
  // heating on / mid/ off
  // TODO

  PCICR |= _BV(PCIE1) | _BV(PCIE2);
}

void setup_screen(Noritake_VFD_GU7000 &s) {
  //
  s.GU7000_init();
}

uint8_t setup_rtc(ds3231_handle_t &ds) {
  memset(&ds, 0, sizeof(ds));
  ds.iic_init = dummy;
  ds.iic_deinit = dummy;
  ds.iic_write = iic_write;
  ds.iic_read = iic_read;
  ds.debug_print = debug_print;
  ds.receive_callback = receive_callback;
  ds.delay_ms = delay_ms;

  int res = ds3231_init(&ds);
  res |= ds3231_set_square_wave(&ds, ds3231_bool_t::DS3231_BOOL_TRUE);

  return res;
}

enum class FanStateE : uint8_t {
  Off,
  On,
};

// Remember to read initial conditions of these
volatile bool UpButtonPressedFlag = false;
volatile bool DownButtonPressedFlag = false;
volatile bool RtcSecondPassed = false;
volatile bool CoolingSelected = false;
volatile bool HeatingSelected = false;
volatile etl::optional<FanStateE> FanStateChangedFlag = etl::nullopt;

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

  gLastPinD = pin;
}