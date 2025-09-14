#ifndef RS_DRIVER_H
#define RS_DRIVER_H

#include <stdint.h>

#include "../data_types.hpp"

extern "C" {

struct AvrDriverCallbacks {
  void (*OnButtonPressed)(Button b);
  void (*OnSerialMessage)(const char* message, uint16_t messageLen);
};

struct ThermoButtonState {
  FanModeT FanState;
  HeatModeT HeatingState;
  ReverseValveModeT ReverseValveState;
};

void DriverInit(const AvrDriverCallbacks& callbacks);
void DriverGetButtonStateNow(ThermoButtonState* data);
void DriverDisplayTemp(uint8_t tempC, TemperatureUnitT unit);
void DriverDisplaySetPoint(uint8_t tempC, TemperatureUnitT);
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