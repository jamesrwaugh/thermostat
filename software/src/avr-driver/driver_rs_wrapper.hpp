#pragma once

#include <stdint.h>

extern "C" {

enum class Button : uint8_t {
  Up,
  Down,
  TempHeat,
  TempCold,
  FanAuto,
  FanOn,
};

struct AvrDriverCallbacks {
  void (*OnButtonPressed)(Button b);
  void (*OnTemperatureRead)(uint8_t degreesC);
  void (*OnSerialMessage)(const char *message, uint16_t messageLen);
  void (*OnSecondPassed)();
};

void DriverInit(const AvrDriverCallbacks &callbacks);
void DriverWriteScreen(const char *message, uint8_t messageLen);
void DriverWriteSerialPort(const uint8_t *bytes, uint8_t numBytes);
}
