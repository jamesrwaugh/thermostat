#include "sim_thermostat.hpp"

#include <simavr/avr_uart.h>
#include <simavr/sim_irq.h>

#include <chrono>
#include <cstring>
#include <memory>
#include <simavr-toolbox/sim_bouncy_switch.hpp>
#include <simavr-toolbox/sim_gu7000.hpp>

void FakeCb(struct avr_irq_t* irq, uint32_t value, void* param) {
  auto cb = (SimAvrThermostat::RelayCb*)param;
  bool on = value != 0;

  if (strcmp(irq->name, "PC0") == 0) {
    (*cb)(Relay::Fan, on);
  } else if (strcmp(irq->name, "PB2") == 0) {
    (*cb)(Relay::Compressor, on);
  } else if (strcmp(irq->name, "PC1") == 0) {
    (*cb)(Relay::Heat, on);
  } else if (strcmp(irq->name, "PC2") == 0) {
    (*cb)(Relay::ReverseValve, on);
  } else {
    std::abort();
  }
}

SimAvrThermostat::SimAvrThermostat(std::string_view filename, bool gdb,
                                   TaskReceiver& receiver)
    : FtxUiSimulatedAvr(filename, gdb), S_{receiver->MakeSender()} {
  // Screen
  Screen = std::make_unique<SimGu7000>(Avr_, get_pin_irq(Avr_, 'B', 1), 0xA0);

  // Buttons
  UpButton = std::make_unique<SimBouncySwitch>(
      *Avr_, *get_pin_irq(Avr_, 'D', 3), false);
  DownButton = std::make_unique<SimBouncySwitch>(
      *Avr_, *get_pin_irq(Avr_, 'D', 2), false);
  SelectButton = std::make_unique<SimBouncySwitch>(
      *Avr_, *get_pin_irq(Avr_, 'C', 3), false);
  ReverseValveSwitch = std::make_unique<SimBouncySwitch>(
      *Avr_, *get_pin_irq(Avr_, 'D', 7), false);
  TempHeat = std::make_unique<SimBouncySwitch>(
      *Avr_, *get_pin_irq(Avr_, 'D', 4), false);
  TempCool = std::make_unique<SimBouncySwitch>(
      *Avr_, *get_pin_irq(Avr_, 'D', 5), false);
  FanSwitch = std::make_unique<SimBouncySwitch>(
      *Avr_, *get_pin_irq(Avr_, 'D', 6), false);

  // TMP116
  Tmp116_ = std::make_unique<SimTMP116>(Avr_, 0x90);

  // RTC
  ds1338_virt_init(Avr_, &Rtc_);
  ds1338_virt_attach_twi(&Rtc_, AVR_IOCTL_TWI_GETIRQ(0));

  // Relay callbacks
  RelayCb_ = std::bind(&SimAvrThermostat::OnRelayChange, this,
                       std::placeholders::_1, std::placeholders::_2);

  // Relay IRQs
  auto x = get_pin_irq(Avr_, 'C', 0);
  avr_irq_register_notify(x, FakeCb, &RelayCb_);
  x = get_pin_irq(Avr_, 'B', 2);
  avr_irq_register_notify(x, FakeCb, &RelayCb_);
  x = get_pin_irq(Avr_, 'C', 1);
  avr_irq_register_notify(x, FakeCb, &RelayCb_);
  x = get_pin_irq(Avr_, 'C', 2);
  avr_irq_register_notify(x, FakeCb, &RelayCb_);
}

void SimAvrThermostat::OnRelayChange(Relay r, bool value) {
  switch (r) {
    case Relay::Fan:
      Relays_.Fan = value;
      break;
    case Relay::Compressor:
      Relays_.Compressor = value;
      break;
    case Relay::Heat:
      Relays_.Heat = value;
      break;
    case Relay::ReverseValve:
      Relays_.ReverseValve = value;
      break;
  }
}

const RelayState& SimAvrThermostat::GetRelayState() const {
  return Relays_;
}

void SimAvrThermostat::PushUpButton() {
  UpButton->CloseForMs(std::chrono::milliseconds(100));
}

void SimAvrThermostat::PushDownButton() {
  DownButton->CloseForMs(std::chrono::milliseconds(100));
}

void SimAvrThermostat::SwitchHeatingHeat() {
  TempCool->Open();
  TempHeat->Close();
}

void SimAvrThermostat::SwitchHeatingCooling() {
  TempHeat->Open();
  TempCool->Close();
}

void SimAvrThermostat::SwitchHeatingNone() {
  TempHeat->Open();
  TempCool->Open();
}

void SimAvrThermostat::SwitchFanOn() {
  FanSwitch->Close();
}

void SimAvrThermostat::SwitchFanAuto() {
  FanSwitch->Open();
}

void SimAvrThermostat::SwitchReverseValve(bool onForHeat) {
  FanSwitch->Set(onForHeat);
}

void SimAvrThermostat::SendSerialMessage(std::string_view message) {
  //
}

void SimAvrThermostat::UpdateSimulatedTemperature() {
  Tmp116_->SimulateTempChange(Relays_);
}

void SimAvrThermostat::RunOnceExtra() {
  UpdateSimulatedTemperature();
}