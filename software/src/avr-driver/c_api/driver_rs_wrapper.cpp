#include "c_api/driver_rs_wrapper.hpp"

#include <avr/io.h>
#include <avr/sleep.h>
#include <driver_ds1307.h>
#include <etl/optional.h>
#include <twi_master.h>

#include "Noritake_VFD_GU7000.h"
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

uint8_t CelsiusToFreedom(uint8_t celsius) {
  uint16_t scratch = celsius;
  scratch = (scratch << 1) - (scratch >> 2) + (scratch >> 4) + 32;
  return scratch & 0xFF;
}

void DriverDisplayTemp(uint8_t tempC, TemperatureUnitT unit) {
  auto displayTemp =
      unit == TemperatureUnitT::Freedom ? CelsiusToFreedom(tempC) : tempC;
  auto& screen = gDriver->Screen;
  {
    AutoTwi t;
    screen.GU7000_setFontSize(2, 2, false);
    screen.print(0, 0, "    ");
    screen.GU7000_setCursor(0, 0);
    screen.print(displayTemp, 10);
    screen.GU7000_setFontSize(1, 1, false);
    screen.print(unit == TemperatureUnitT::Freedom ? 'F' : 'C');
  }
  {
    AutoTwi t;
    screen.GU7000_setFontSize(2, 2, false);
    screen.print(30, 0, "   ");
    screen.GU7000_setCursor(30, 0);
    screen.print(displayTemp, 10);
    screen.GU7000_setFontSize(1, 1, false);
    screen.print("RH");
  }
}

void DriverDisplaySetPoint(uint8_t tempC, TemperatureUnitT unit) {
  auto displayTemp =
      unit == TemperatureUnitT::Freedom ? CelsiusToFreedom(tempC) : tempC;
  AutoTwi t;
  auto& screen = gDriver->Screen;
  screen.GU7000_setCursor(80, 0);
  screen.print("S@ ");
  screen.print(displayTemp, 10);
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
