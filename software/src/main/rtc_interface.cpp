#include "rtc_interface.hpp"
#include <stdint.h>
#include <twi_master.h>

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

void delay_ms(uint32_t ms) {
  // Only used for reading temp, which we do not do, so get out of jail free
  // card.
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