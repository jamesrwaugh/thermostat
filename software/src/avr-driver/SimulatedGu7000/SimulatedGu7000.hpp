#pragma once

#include <stdint.h>

class SimulatedGu7000 {
 public:
  SimulatedGu7000(uint8_t busyPin) {}
  static const uint8_t FakeI2cAddress = 0x64;
  void DriverDisplayTemp(uint8_t temp);
  void DriverDisplaySetPoint(uint8_t temp);
  void DriverDisplayIsHeating();
  void DriverDisplayIsCooling();
  void DriverDisplayIsIdle();
};