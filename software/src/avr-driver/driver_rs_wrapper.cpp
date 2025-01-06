#include "driver_rs_wrapper.hpp"

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/sleep.h>
#include <etl/optional.h>

#include "driver.hpp"

extern "C" {

void DriverInit(const AvrDriverCallbacks &callbacks, void *userData) {
  gDriver.emplace(callbacks, userData);
  gDriver->Setup();
}

void DriverWriteSerialPort(const uint8_t *bytes, uint8_t numBytes) {
  gDriver->Serial_.write(bytes, numBytes);
}

void DriverDisplayTemp(uint8_t temp) {
  auto &screen = gDriver->Screen;
  screen.GU7000_selectWindow(AvrDrivers::Window::LowerRight);
  screen.GU7000_clearScreen();
  screen.GU7000_setCursor(0, 0);
  screen.print(temp);
}

void DriverDisplaySetPoint(uint8_t temp) {
  auto &screen = gDriver->Screen;
  screen.GU7000_selectWindow(AvrDrivers::Window::UpperRight);
  screen.GU7000_clearScreen();
  screen.GU7000_setCursor(0, 0);
  screen.print(temp);
}

void DriverDisplayIsHeating() {
  auto &screen = gDriver->Screen;
  screen.GU7000_selectWindow(AvrDrivers::Window::LowerLeft);
  screen.GU7000_clearScreen();
  screen.GU7000_setCursor(0, 0);
  screen.print("HEAT");
}

void DriverDisplayIsCooling() {
  auto &screen = gDriver->Screen;
  screen.GU7000_selectWindow(AvrDrivers::Window::LowerLeft);
  screen.GU7000_clearScreen();
  screen.GU7000_setCursor(0, 0);
  screen.print("COOL");
}

uint8_t DriverReadTemp() {
  return gDriver->TempSensor.read_temp();
}

void DriverDisplayIsIdle() {
  auto &screen = gDriver->Screen;
  screen.GU7000_selectWindow(AvrDrivers::Window::LowerLeft);
  screen.GU7000_clearScreen();
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
  sleep_cpu();
}

//
}
