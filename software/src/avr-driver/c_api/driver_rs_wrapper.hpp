#ifndef RS_DRIVER_H
#define RS_DRIVER_H

#include <ThermoSaveData_bp.h>
#include <stdint.h>

#include "../data_types.hpp"

struct ds1307_time_s;
class Noritake_VFD_GU7000;

extern "C" {

struct AvrDriverCallbacks {
  void (*OnButtonPressed)(Button b);
};

struct ThermoButtonState {
  FanModeT FanState;
  HeatModeT HeatingState;
  ReverseValveModeT ReverseValveState;
};

void DriverInit();
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
int8_t DriverReadButton();
bool DriverSetTime(const ds1307_time_s& time);
bool DriverSetTimeFromSaveData(const Time& time, const Date& date);
bool DriverGetTime(ds1307_time_s& time);
bool DriverGetSerialByte(uint8_t* byte);
bool DriverSaveData(const ThermoSaveData& data);
bool DriverLoadData(ThermoSaveData& data);
}

Noritake_VFD_GU7000& DriverGetScreenHandle();

struct AutoTwi final {
  static uint8_t instanceCount_;
  static constexpr uint8_t Gu7000SlaveAddr_ = 0x50;
  AutoTwi();
  ~AutoTwi();
};

#endif