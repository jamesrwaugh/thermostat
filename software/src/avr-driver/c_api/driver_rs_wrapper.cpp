#include "c_api/driver_rs_wrapper.hpp"

#include <avr/io.h>
#include <avr/sleep.h>
#include <driver_ds1307.h>
#include <etl/optional.h>
#include <twi_master.h>

#include "Noritake_VFD_GU7000.h"
#include "c_api/temperature.hpp"
#include "driver.hpp"

extern "C" {

void DriverInit() {
  gDriver.emplace();
  gDriver->Setup();
}

void DriverGetButtonStateNow(ThermoButtonState* data) {
  gDriver->ReadStateNow(data);
}

void DriverWriteSerialPort(const uint8_t* bytes, uint8_t numBytes) {
  gDriver->Serial_.write(bytes, numBytes);
  gDriver->Serial_.write("\r\n");
}

void DriverWriteSerialPortS(const char* message) {
  gDriver->Serial_.write(message);
  gDriver->Serial_.write("\r\n");
}

void DriverWriteSerialPortRaw(const uint8_t* bytes, uint8_t numBytes) {
  gDriver->Serial_.write(bytes, numBytes);
  gDriver->Serial_.flush();
}

void DriverWriteSerialPortRawCh(char ch) {
  gDriver->Serial_.write(ch);
}

bool DriverGetSerialByte(uint8_t* byte) {
  int8_t b = gDriver->Serial_.read();
  if (b == -1) {
    return false;
  }
  *byte = b;
  return true;
}

uint8_t DriverGetSerialAvailableBytes() {
  return gDriver->Serial_.available();
}

const uint8_t* const DriverGetSerialRxBuffer() {
  return gDriver->Serial_.rx_buffer();
}

bool DriverSaveData(const ThermoSaveData& data) {
  return gDriver->SaveData(data);
}

bool DriverLoadData(ThermoSaveData& data) {
  return gDriver->LoadData(data);
}

void DriverDisplayTemp(const Temperature& tp, const Humidity& hum,
                       TemperatureUnitT unit) {
  auto& screen = gDriver->Screen;
  {
    AutoTwi t;
    screen.GU7000_setFontSize(2, 2, false);
    screen.print(0, 0, "    ");
    screen.GU7000_setCursor(0, 0);
    screen.print(tp.GetUnitWhole(unit), 10);
    screen.GU7000_setFontSize(1, 1, false);
    screen.print(unit == TemperatureUnitT::Freedom ? 'F' : 'C');
  }
  {
    AutoTwi t;
    screen.GU7000_setFontSize(2, 2, false);
    screen.print(30, 0, "   ");
    screen.GU7000_setCursor(30, 0);
    screen.print(hum.ToPercent(), 10);
    screen.GU7000_setFontSize(1, 1, false);
    screen.print("%RH");
  }
}

void DriverDisplaySetPoint(const Temperature& temp, TemperatureUnitT unit) {
  AutoTwi t;
  auto& screen = gDriver->Screen;
  screen.GU7000_setCursor(80, 0);
  screen.print("S@ ");
  screen.print(temp.GetUnitWhole(unit), 10);
}

void DriverDisplayIsHeating() {
  AutoTwi t;
  auto& screen = gDriver->Screen;
  screen.print(76, 8, "Heating");
}

void DriverDisplayIsCooling() {
  AutoTwi t;
  auto& screen = gDriver->Screen;
  screen.print(76, 8, "Cooling");
}

void DriverDisplayIsIdle() {
  AutoTwi t;
  auto& screen = gDriver->Screen;
  screen.print(91, 8, "Idle");
}

void DriverDisplayClearScreen() {
  AutoTwi t;
  gDriver->Screen.GU7000_clearScreen();
}

void DriverReadRawTemp(uint16_t& outTempTicks, uint16_t& outHumidityTicks) {
  SHT4x::Reading r = gDriver->TempSensor.ReadHighPrecision();
  outTempTicks = r.TemperatureTicks;
  outHumidityTicks = r.HumidityTicks;
}

void DriverRelayOn(Relay r) {
  gDriver->RelayOn(r);
}

void DriverRelayOff(Relay r) {
  gDriver->RelayOff(r);
}

void DriverMcuSleep() {
  SMCR |= _BV(SE);
  sleep_cpu();
  SMCR &= ~_BV(SE);
}

int8_t DriverReadButton() {
  return gDriver->ReadInput();
}

bool DriverSetTime(const ds1307_time_s& time) {
  return ds1307_set_time(&gDriver->Rtc, const_cast<ds1307_time_s*>(&time)) == 0;
}

bool DriverGetTime(ds1307_time_s& time) {
  return ds1307_get_time(&gDriver->Rtc, &time) == 0;
}
}

Noritake_VFD_GU7000& DriverGetScreenHandle() {
  return gDriver->Screen;
}

uint8_t AutoTwi::instanceCount_ = 0;

AutoTwi::AutoTwi() {
  if (instanceCount_ == 0) {
    tw_master_setup_transmit(Gu7000SlaveAddr_);
  }
  instanceCount_ += 1;
}

AutoTwi::~AutoTwi() {
  instanceCount_ -= 1;
  if (instanceCount_ == 0) {
    tw_master_end_transmit();
  }
}
