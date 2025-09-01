#ifndef RS_DRIVER_H
#define RS_DRIVER_H

#include <stdint.h>

extern "C" {

enum class Button : uint8_t {
  Up = 0,
  Down,
  Select,
  TempHeat,
  TempCold,
  TempNone,
  FanAuto,
  FanOn,
};

enum class Relay : uint8_t {
  Fan = 0,
  Compressor,
  Heat,
  ReversingValve,
};

struct AvrDriverCallbacks {
  void (*OnButtonPressed)(Button b, void* userData);
  void (*OnSerialMessage)(const char* message, uint16_t messageLen,
                          void* userData);
};

enum class ReverseValveTypeE {
  EnergizeToHeat = 0,
  EnergizeToCool = 1,
};

struct ThermostatData {
  ReverseValveTypeE ReverseValveType;
};

void DriverInit(const AvrDriverCallbacks& callbacks, void* userData);
void DriverGetThermostatType(ThermostatData* data);
void DriverDisplayTemp(uint8_t temp);
void DriverDisplaySetPoint(uint8_t temp);
void DriverDisplayIsHeating();
void DriverDisplayIsCooling();
uint8_t DriverReadTemp();
void DriverDisplayIsIdle();
void DriverRelayOn(Relay r);
void DriverRelayOff(Relay r);
void DriverWriteSerialPort(const uint8_t* bytes, uint8_t numBytes);
void DriverMcuSleep();
void DriverPollInput();
uint8_t DriverWriteFlash(uint8_t address, uint8_t* data, uint8_t length);
uint8_t DriverReadFlash(uint8_t address, uint8_t* buffer, uint8_t maxLength);
}

#endif