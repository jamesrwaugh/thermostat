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

enum class Relay : uint8_t {
  Fan,
  Compressor,
  Heat,
  ReversingValve,
};

struct AvrDriverCallbacks {
  void (*OnButtonPressed)(Button b, void *userData);
  void (*OnSerialMessage)(const char *message, uint16_t messageLen,
                          void *userData);
  void (*OnSecondPassed)(void *userData);
};

void DriverInit(const AvrDriverCallbacks &callbacks, void *userData);
void DriverDisplayTemp(uint8_t temp);
void DriverDisplaySetPoint(uint8_t temp);
void DriverDisplayIsHeating();
void DriverDisplayIsCooling();
uint8_t DriverReadTemp();
void DriverDisplayIsIdle();
void DriverRelayOn(Relay r);
void DriverRelayOff(Relay r);
void DriverWriteSerialPort(const uint8_t *bytes, uint8_t numBytes);
}
