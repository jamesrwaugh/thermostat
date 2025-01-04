#pragma once

#include <stdint.h>

extern "C" {

enum class Button : uint8_t {
  Up,
  Down,
  TempHeat,
  TempCold,
  TempNone,
  FanAuto,
  FanOn,
};

struct AvrDriverCallbacks {
  void (*OnButtonPressed)(Button b, void *userData);
  void (*OnTemperatureRead)(uint8_t degreesC, void *userData);
  void (*OnSerialMessage)(const char *message, uint16_t messageLen,
                          void *userData);
  void (*OnSecondPassed)(void *userData);
};

void DriverInit(const AvrDriverCallbacks &callbacks, void *userData);
void DriverWriteScreen(const char *message, uint8_t messageLen);
void DriverWriteSerialPort(const uint8_t *bytes, uint8_t numBytes);
}
