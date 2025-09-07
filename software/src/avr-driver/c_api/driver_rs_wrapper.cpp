#include "c_api/driver_rs_wrapper.hpp"

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/sleep.h>
#include <driver_ds1307.h>
#include <etl/optional.h>

#include "driver.hpp"

extern "C" {

void DriverInit(const AvrDriverCallbacks& callbacks, void* userData) {
  gDriver.emplace(callbacks, userData);
  gDriver->Setup();
}

void DriverGetButtonStateNow(ThermoButtonState* data) {
  gDriver->ReadStateNow(data);
}

void DriverWriteSerialPort(const uint8_t* bytes, uint8_t numBytes) {
  gDriver->Serial_.write(bytes, numBytes);
}

void DriverWriteSerialPortLine(const char* message) {
  gDriver->Serial_.write(message);
  gDriver->Serial_.write("\n");
}

void DriverDisplayTemp(uint8_t temp) {
#ifdef SIMULATED
  auto& screen = gDriver->Screen;
  screen.DriverDisplayTemp(temp);
#else
  auto& screen = gDriver->Screen;
  screen.GU7000_selectWindow(AvrDrivers::Gu7kWindowId::LowerRight);
  screen.GU7000_clearScreen();
  screen.GU7000_setCursor(0, 0);
  screen.print(temp);
#endif
}

void DriverDisplaySetPoint(uint8_t temp) {
#ifdef SIMULATED
  auto& screen = gDriver->Screen;
  screen.DriverDisplaySetPoint(temp);
#else
  auto& screen = gDriver->Screen;
  screen.GU7000_selectWindow(AvrDrivers::Gu7kWindowId::UpperRight);
  screen.GU7000_clearScreen();
  screen.GU7000_setCursor(0, 0);
  screen.print(temp);
#endif
}

void DriverDisplayIsHeating() {
#ifdef SIMULATED
  auto& screen = gDriver->Screen;
  screen.DriverDisplayIsHeating();
#else
  auto& screen = gDriver->Screen;
  screen.GU7000_selectWindow(AvrDrivers::Gu7kWindowId::LowerLeft);
  screen.GU7000_clearScreen();
  screen.GU7000_setCursor(0, 0);
  screen.print("HEAT");
#endif
}

void DriverDisplayIsCooling() {
#ifdef SIMULATED
  auto& screen = gDriver->Screen;
  screen.DriverDisplayIsCooling();
#else
  auto& screen = gDriver->Screen;
  screen.GU7000_selectWindow(AvrDrivers::Gu7kWindowId::LowerLeft);
  screen.GU7000_clearScreen();
  screen.GU7000_setCursor(0, 0);
  screen.print("COOL");
#endif
}

void DriverDisplayIsIdle() {
#ifdef SIMULATED
  auto& screen = gDriver->Screen;
  screen.DriverDisplayIsIdle();
#else
  auto& screen = gDriver->Screen;
  screen.GU7000_selectWindow(AvrDrivers::Gu7kWindowId::LowerLeft);
  screen.GU7000_clearScreen();
#endif
}

uint8_t DriverReadTemp() {
  return gDriver->TempSensor.read_temp();
}

void DriverRelayOn(Relay r) {
  switch (r) {
    case Relay::Fan:
      // PC0
      PORTC |= _BV(PORTC0);
      break;
    case Relay::Compressor:
      // PB2
      PORTB |= _BV(PORTB2);
      break;
    case Relay::Heat:
      // PC1
      PORTC |= _BV(PORTC1);
      break;
    case Relay::ReversingValve:
      // PC2
      PORTC |= _BV(PORTC2);
      break;
  }
}

void DriverRelayOff(Relay r) {
  switch (r) {
    case Relay::Fan:
      // PC0
      PORTC &= ~_BV(PORTC0);
      break;
    case Relay::Compressor:
      // PB2
      PORTB &= ~_BV(PORTB2);
      break;
    case Relay::Heat:
      // PC1
      PORTC &= ~_BV(PORTC1);
      break;
    case Relay::ReversingValve:
      // PC2
      PORTC &= ~_BV(PORTC2);
      break;
  }
}

void DriverMcuSleep() {
  SMCR |= _BV(SE);
  sleep_cpu();
  SMCR &= ~_BV(SE);
}

void DriverPollInput() {
  gDriver->ReadInput();
}

uint8_t DriverWriteFlash(uint8_t address, uint8_t* data, uint8_t length) {
  return ds1307_write_ram(&gDriver->Rtc, address, data, length);
}

uint8_t DriverReadFlash(uint8_t address, uint8_t* buffer, uint8_t maxLength) {
  return ds1307_read_ram(&gDriver->Rtc, address, buffer, maxLength);
}
}
