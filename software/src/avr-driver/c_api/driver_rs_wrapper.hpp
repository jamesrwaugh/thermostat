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
  ReverseValveOnHeat,
  ReverseValveOnCool,
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

enum class FanModeT : uint8_t {
  On = 0,
  Auto = 1,
};

enum class HeatModeT : uint8_t {
  Heating = 0,
  Cooling = 1,
  None = 2,
};

enum class ReverseValveModeT : uint8_t {
  OnForHeating = 0,
  OnForCooling = 1,
};

struct ThermoButtonState {
  FanModeT FanState;
  HeatModeT HeatingState;
  ReverseValveModeT ReverseValveState;
};

void DriverInit(const AvrDriverCallbacks& callbacks, void* userData);
void DriverGetButtonStateNow(ThermoButtonState* data);
void DriverDisplayTemp(uint8_t temp);
void DriverDisplaySetPoint(uint8_t temp);
void DriverDisplayIsHeating();
void DriverDisplayIsCooling();
void DriverDisplayIsIdle();
uint8_t DriverReadTemp();
void DriverRelayOn(Relay r);
void DriverRelayOff(Relay r);
void DriverWriteSerialPortRaw(const uint8_t* bytes, uint8_t numBytes);
void DriverWriteSerialPortRawCh(char ch);
void DriverWriteSerialPort(const uint8_t* bytes, uint8_t numBytes);
void DriverWriteSerialPortS(const char* message);
void DriverMcuSleep();
void DriverPollInput();
uint8_t DriverWriteFlash(uint8_t address, uint8_t* data, uint8_t length);
uint8_t DriverReadFlash(uint8_t address, uint8_t* buffer, uint8_t maxLength);
}

#endif