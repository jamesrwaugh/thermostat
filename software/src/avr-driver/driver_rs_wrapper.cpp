#include "driver_rs_wrapper.hpp"
#include "driver.hpp"
#include <etl/optional.h>

etl::optional<AvrDrivers> gDriver;

extern "C" {

void DriverInit(const AvrDriverCallbacks &callbacks, void *userData) {
  gDriver.emplace(callbacks, userData);
  gDriver->Setup();
}

void DriverWriteSerialPort(const uint8_t *bytes, uint8_t numBytes) {
  gDriver->Serial_.write(bytes, numBytes);
}

void DriverDisplayTemp(uint8_t temp) {
  //
}

void DriverDisplayIsHeating() {
  //
}

void DriverDisplayIsCooling() {
  //
}

uint8_t DriverReadTemp() {
  //
  return 0;
}

void DriverDisplayIsIdle() {
  //
}

void DriverRelayOn(Relay r) {
  //
}

void DriverRelayOff(Relay r) {
  //
}

//
}
