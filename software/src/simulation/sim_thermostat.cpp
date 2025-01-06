#include "sim_thermostat.hpp"

#include <simavr/avr_uart.h>

#include <memory>

#include "simavr-toolbox/sim_bouncy_switch.hpp"
#include "simavr-toolbox/sim_gu7000.hpp"

SimAvrThermostat::SimAvrThermostat(std::string_view filename, bool gdb)
    : FtxUiSimulatedAvr(filename, gdb) {
  Screen = std::make_unique<SimGu7000>(Avr_, get_pin_irq(Avr_, 'B', 1), 0xA0);
  UpButton =
      std::make_unique<SimBouncySwitch>(*Avr_, *get_pin_irq(Avr_, 'D', 3));
  DownButton =
      std::make_unique<SimBouncySwitch>(*Avr_, *get_pin_irq(Avr_, 'D', 2));
  TempHeat =
      std::make_unique<SimBouncySwitch>(*Avr_, *get_pin_irq(Avr_, 'D', 4));
  TempCool =
      std::make_unique<SimBouncySwitch>(*Avr_, *get_pin_irq(Avr_, 'D', 5));
  FanIsOn =
      std::make_unique<SimBouncySwitch>(*Avr_, *get_pin_irq(Avr_, 'D', 6));

  // disable the stdio dump for serial 1, as we have a TUI output
  uint32_t flags1 = 0;
  avr_ioctl(Avr_, AVR_IOCTL_UART_GET_FLAGS('0'), &flags1);
  flags1 &= ~AVR_UART_FLAG_STDIO;
  avr_ioctl(Avr_, AVR_IOCTL_UART_SET_FLAGS('0'), &flags1);
}

void SimAvrThermostat::PushUpButton() {
  //
}

void SimAvrThermostat::PushDownButton() {
  //
}

void SimAvrThermostat::SwitchHeatingHeat() {
  //
}

void SimAvrThermostat::SwitchHeatingCooling() {
  //
}

void SimAvrThermostat::SwitchHeatingNone() {
  //
}

void SimAvrThermostat::SwitchFanOn() {
  //
}

void SimAvrThermostat::SwitchFanAuto() {
  //
}

void SimAvrThermostat::SendSerialMessage(std::string_view message) {
  //
}
