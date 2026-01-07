#include "c_api/driver_rs_wrapper.hpp"

#include <avr/io.h>
#include <avr/sleep.h>
#include <driver_ds1307.h>
#include <etl/optional.h>

#include "driver.hpp"
#include "twi_master.h"

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

bool DriverSaveData(const ThermoSaveData& data) {
  return gDriver->SaveData(data);
}

bool DriverLoadData(ThermoSaveData& data) {
  return gDriver->LoadData(data);
}

uint8_t CelsiusToFreedom(uint8_t celsius) {
  uint16_t scratch = celsius;
  scratch = (scratch << 1) - (scratch >> 2) + (scratch >> 4) + 32;
  return scratch & 0xFF;
}

void DriverDisplayTemp(uint8_t tempC, TemperatureUnitT unit) {
  auto displayTemp =
      unit == TemperatureUnitT::Freedom ? CelsiusToFreedom(tempC) : tempC;
#if SIMULATED == 1
  gDriver->Screen.DriverDisplayTemp(displayTemp, unit);
#else
  auto& screen = gDriver->Screen;
  // screen.GU7000_selectWindow(AvrDrivers::Gu7kWindowId::LowerRight);
  screen.GU7000_clearScreen();
  screen.GU7000_setCursor(0, 0);
  screen.print(displayTemp);
#endif
}

void DriverDisplaySetPoint(uint8_t tempC, TemperatureUnitT unit) {
  auto displayTemp =
      unit == TemperatureUnitT::Freedom ? CelsiusToFreedom(tempC) : tempC;
#if SIMULATED == 1
  gDriver->Screen.DriverDisplaySetPoint(displayTemp, unit);
#else
  auto& screen = gDriver->Screen;
  // screen.GU7000_selectWindow(AvrDrivers::Gu7kWindowId::UpperRight);
  screen.GU7000_clearScreen();
  screen.GU7000_setCursor(0, 0);
  screen.print(displayTemp);
#endif
}

void DriverDisplayIsHeating() {
#if SIMULATED == 1
  auto& screen = gDriver->Screen;
  screen.DriverDisplayIsHeating();
#else
  auto& screen = gDriver->Screen;
  // screen.GU7000_selectWindow(AvrDrivers::Gu7kWindowId::LowerLeft);
  screen.GU7000_clearScreen();
  screen.GU7000_setCursor(0, 0);
  screen.print("HEAT");
#endif
}

void DriverDisplayIsCooling() {
#if SIMULATED == 1
  auto& screen = gDriver->Screen;
  screen.DriverDisplayIsCooling();
#else
  auto& screen = gDriver->Screen;
  // screen.GU7000_selectWindow(AvrDrivers::Gu7kWindowId::LowerLeft);
  screen.GU7000_clearScreen();
  screen.GU7000_setCursor(0, 0);
  screen.print("COOL");
#endif
}

void DriverDisplayIsIdle() {
#if SIMULATED == 1
  auto& screen = gDriver->Screen;
  screen.DriverDisplayIsIdle();
#else
  auto& screen = gDriver->Screen;
  // screen.GU7000_selectWindow(AvrDrivers::Gu7kWindowId::LowerLeft);
  screen.GU7000_clearScreen();
#endif
}

uint8_t DriverReadTemp() {
  return gDriver->TempSensor.ReadTempC();
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

bool DriverSetTimeFromSaveData(const Time& time, const Date& date) {
  ds1307_time_s timeStruct{
      .year = date.year,
      .month = date.month,
      .week = date.day_of_week,
      .date = date.day,
      .hour = time.hour,
      .minute = time.minute,
      .second = time.second,
      .am_pm = time.am_pm == TIME_AM ? ds1307_am_pm_t::DS1307_AM
                                     : ds1307_am_pm_t::DS1307_PM,
  };
  return DriverSetTime(timeStruct);
}

bool DriverGetTime(ds1307_time_s& time) {
  return ds1307_get_time(&gDriver->Rtc, &time) == 0;
}
}

Noritake_VFD_GU7000& DriverGetScreenHandle() {
  return gDriver->Screen;
}

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
