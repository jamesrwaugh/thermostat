#include "driver_rs_wrapper.hpp"
#include "driver.hpp"
#include <etl/optional.h>

etl::optional<AvrDrivers> gDriver;

extern "C" {

void DriverInit(const AvrDriverCallbacks &callbacks, void *userData) {
  gDriver.emplace(callbacks, userData);
  gDriver->Setup();
}

void DriverWriteScreen(const char *message, uint8_t messageLen) {
  gDriver->Screen.print(message, messageLen);
}

void DriverWriteSerialPort(const uint8_t *bytes, uint8_t numBytes) {
  gDriver->Serial_.write(bytes, numBytes);
}

//
}
