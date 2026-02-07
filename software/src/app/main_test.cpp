#include <avr/io.h>

#include "driver_rs_wrapper.hpp"
#include "temperature.hpp"

int main() {
  DriverInit();

  while (1) {
    Temperature t;
    uint8_t byte;
    DriverGetSerialByte(&byte);
    Temperature t2;
    DriverGetSerialByte(&byte);
    t2.SetFromSht4xSensor(byte);
    t.SetFromSht4xSensor(byte);
    t.ChangeBy1Unit(TemperatureUnitT::Celsius, true);
    t.ChangeBy1Unit(TemperatureUnitT::Freedom, false);
    auto x = t.GetUnitWhole(TemperatureUnitT::Celsius);
    auto y = t.GetUnitWhole(TemperatureUnitT::Freedom);
    DriverWriteSerialPortRawCh(x + y);
    DriverWriteSerialPortRawCh(t <= t2 ? 1 : 0);
  }

  return 0;
}
