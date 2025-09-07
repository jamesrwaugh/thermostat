#include "sim_thermostat.hpp"

#include <simavr/avr_uart.h>
#include <simavr/sim_irq.h>

#include <chrono>
#include <cstring>
#include <memory>
#include <simavr-toolbox/sim_base.hpp>
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
    : FtxUiSimulatedAvr(filename, gdb, receiver) {
  // Screen
  Screen = std::make_unique<SimFakeGu7000>(Avr_, 0xA0);

  // Buttons
  UpButton =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('D', 3), false);
  DownButton =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('D', 2), false);
  SelectButton =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('D', 7), false);
  ReverseValveSwitch =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('C', 3), false);
  TempHeat =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('D', 4), false);
  TempCool =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('D', 5), false);
  FanSwitch =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('D', 6), false);

  // TMP116
  Tmp116_ = std::make_unique<SimTMP116>(Avr_, 0x90);

  // RTC
  ds1338_virt_init(Avr_, &Rtc_);
  ds1338_virt_attach_twi(&Rtc_, AVR_IOCTL_TWI_GETIRQ(0));

  // Relay callbacks
  RelayCb_ = std::bind(&SimAvrThermostat::OnRelayChange, this,
                       std::placeholders::_1, std::placeholders::_2);

  // Relay IRQs
  avr_irq_register_notify(GetPinIrq('C', 0), FakeCb, &RelayCb_);
  avr_irq_register_notify(GetPinIrq('B', 2), FakeCb, &RelayCb_);
  avr_irq_register_notify(GetPinIrq('C', 1), FakeCb, &RelayCb_);
  avr_irq_register_notify(GetPinIrq('C', 2), FakeCb, &RelayCb_);
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

const SimFakeGu7000::State& SimAvrThermostat::GetScreenState() const {
  return Screen->GetState();
}

void SimAvrThermostat::PushUpButton() {
  Post([this]() { UpButton->CloseForMs(std::chrono::milliseconds(100)); });
}

void SimAvrThermostat::PushDownButton() {
  Post([this]() { DownButton->CloseForMs(std::chrono::milliseconds(100)); });
}

void SimAvrThermostat::PushSelectButton() {
  Post([this]() { SelectButton->CloseForMs(std::chrono::milliseconds(100)); });
}

void SimAvrThermostat::SwitchHeatingHeat() {
  Post([this]() { TempCool->Open(); });
  Post([this]() { TempHeat->Close(); });
}

void SimAvrThermostat::SwitchHeatingCooling() {
  Post([this]() { TempHeat->Open(); });
  Post([this]() { TempCool->Close(); });
}

void SimAvrThermostat::SwitchHeatingNone() {
  Post([this]() { TempHeat->Open(); });
  Post([this]() { TempCool->Open(); });
}

void SimAvrThermostat::SwitchFanOn() {
  Post([this]() { FanSwitch->Close(); });
}

void SimAvrThermostat::SwitchFanAuto() {
  Post([this]() { FanSwitch->Open(); });
}

void SimAvrThermostat::SwitchReverseValve(bool onForHeat) {
  Post([this, onForHeat]() { FanSwitch->Set(onForHeat); });
}

void SimAvrThermostat::SendSerialMessage(std::string_view message) {
  //
}

void SimAvrThermostat::BeforeAvrCycleSideEffect() {
  Tmp116_->SimulateTempChange(Relays_);
}