#include <Noritake_VFD_GU7000.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <driver_ds3231.h>
#include <stdint.h>
#include <tmp116.h>
#include <twi_master.h>
#include <util/delay.h>

extern "C" {
#include "thermo.h"
}

uint8_t iic_init() {
  // set pullups for SDA / SCL
  DDRC &= ~_BV(PC1);
  DDRC &= ~_BV(PC0);
  PORTC |= _BV(PC1) | _BV(PC0);

  // initialize twi prescaler and bit rate (250k)
  // Set prescaler value of 1
  TWSR &= ~_BV(TWPS0);
  TWSR &= ~_BV(TWPS1);
  TWBR = ((F_CPU / 50000) - 16) / 2;

  return 0;
}

uint8_t iic_init_dummy() {
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
  while (PINB & _BV(1)) {
    ;
  }
  tw_write(data);
}

void hardReset() {}

void setup() {
  ds3231_handle_t ds;
  memset(&ds, 0, sizeof(ds));
  ds.iic_init = iic_init_dummy;
  ds.iic_deinit = iic_init;
  ds.iic_write = iic_write;
  ds.iic_read = iic_read;
  ds.debug_print = debug_print;
  ds.receive_callback = receive_callback;
  ds.delay_ms = delay_ms;

  int res = ds3231_init(&ds);
  res |= ds3231_set_square_wave(&ds, ds3231_bool_t::DS3231_BOOL_TRUE);

  if (res != 0) {
    return;
  }

  Noritake_VFD_GU7000 vfd(13);
  vfd.GU7000_init();

  tmp116_init();
}

int main() {
  iic_init();
  setup();

  CHardwareDrivers d;
  d.read_temp_c_function = nullptr;

  ThermoInit(&d);
  uint8_t set = ThermoGetSetPoint();
  ThermoUpButtonPressed();
  ThermoSecondPassed();

  return set;
}
