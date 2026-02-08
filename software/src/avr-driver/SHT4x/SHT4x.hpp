#pragma once

#include <I2C/twi/twi_master.h>
#include <stdint.h>
#include <util/delay.h>

class SHT4x {
 public:
  static constexpr uint8_t gI2CSlaveAddress = 0x44;
  static constexpr uint8_t gReadHighPrecisionCommand = 0xFD;
  static constexpr uint8_t gActivateHeater20mW1sCommand = 0x1E;

  struct Reading {
    Reading(uint16_t temp, uint16_t humid)
        : TemperatureTicks{temp}, HumidityTicks{humid} {}
    const uint16_t TemperatureTicks;
    const uint16_t HumidityTicks;
  };

  Reading ReadHighPrecision() {
    tw_master_transmit_one(gI2CSlaveAddress, gReadHighPrecisionCommand, true);
    _delay_us(10);
    uint8_t buffer[6];
    tw_master_receive(gI2CSlaveAddress, buffer, sizeof(buffer));
    Reading r(static_cast<uint16_t>(buffer[0] << 8 | buffer[1]),
              static_cast<uint16_t>(buffer[3] << 8 | buffer[4]));
    return r;
  }

  void ActivateHeater() {
    tw_master_transmit_one(gI2CSlaveAddress, gActivateHeater20mW1sCommand,
                           false);
  }
};
