#ifndef RS_DRIVER_H
#define RS_DRIVER_H

#include <ThermoSaveData_bp.h>
#include <stdint.h>

#include "thermo_data_types.hpp"

struct ds1307_time_s;
class Noritake_VFD_GU7000;

extern "C" {

void DriverInit();
void DriverDisplayClearScreen();
void DriverReadRawTemp(uint16_t& outTempTicks, uint16_t& outHumidityTicks);
void DriverRelayOn(Relay r);
void DriverRelayOff(Relay r);
void DriverWriteSerialPortRaw(const uint8_t* bytes, uint8_t numBytes);
void DriverWriteSerialPortRawCh(char ch);
void DriverWriteSerialPort(const uint8_t* bytes, uint8_t numBytes);
void DriverWriteSerialPortS(const char* message);
bool DriverGetSerialByte(uint8_t* byte);
uint8_t DriverGetSerialAvailableBytes();
const uint8_t* const DriverGetSerialRxBuffer();
void DriverMcuSleep();
int8_t DriverReadButton();
bool DriverSetTime(const ds1307_time_s& time);
bool DriverGetTime(ds1307_time_s& time);
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
