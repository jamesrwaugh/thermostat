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

  if (strcmp(irq->name, "=avr.portc.pin0") == 0) {
    (*cb)(Relay::Fan, on);
  } else if (strcmp(irq->name, "=avr.portb.pin2") == 0) {
    (*cb)(Relay::Compressor, on);
  } else if (strcmp(irq->name, "=avr.portc.pin1") == 0) {
    (*cb)(Relay::Heat, on);
  } else if (strcmp(irq->name, "=avr.portc.pin2") == 0) {
    (*cb)(Relay::ReverseValve, on);
  } else {
    std::abort();
  }
}

SimAvrThermostat::SimAvrThermostat(std::string_view filename, bool gdb,
                                   TaskReceiver& receiver)
    : FtxUiSimulatedAvr(filename, gdb, receiver) {
  // Screen
  Screen = std::make_unique<SimGu7000I2C>(Avr_);

  // Buttons
  UpButton =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('D', 3), false);
  DownButton =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('D', 2), false);
  SelectButton =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('D', 5), false);
  ReverseValveSwitch =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('C', 3), false);
  HeatButton =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('D', 7), false);
  FanButton =
      std::make_unique<SimBouncySwitch>(*Avr_, *GetPinIrq('D', 6), false);

  // TMP116
  Tmp116_ = std::make_unique<SimTMP116>(Avr_);

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

  // EEPROM
  Eeprom_ = std::make_unique<Sim47LXX>(Avr_, 1, 1);
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

avr_t* SimAvrThermostat::GetAvr() const {
  return Avr_;
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

void SimAvrThermostat::PushFanButton() {
  Post([this]() { FanButton->CloseForMs(std::chrono::milliseconds(100)); });
}

void SimAvrThermostat::PushHeatButton() {
  Post([this]() { HeatButton->CloseForMs(std::chrono::milliseconds(100)); });
}

void SimAvrThermostat::SwitchReverseValve(bool onForHeat) {
  Post([this, onForHeat]() { ReverseValveSwitch->Set(onForHeat); });
}

void SimAvrThermostat::SendSerialMessage(std::string_view message) {
  //
}

void SimAvrThermostat::BeforeAvrCycleSideEffect() {
  auto now = std::chrono::steady_clock::now();
  auto last_ms_delta =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - LastMsTick_);

  if (last_ms_delta > std::chrono::milliseconds(1)) {
    LastMsTick_ = now;
    Screen->OnMillisecondPassed();
  }

  Tmp116_->SimulateTempChange(Relays_);
}

void SimAvrThermostat::OnUartByteReceived(int uartNumber, uint8_t byte) {
  sim_debug_log("[%d] %d", uartNumber, (int)byte);
}

const SimGu7000Real::DisplayMemory& SimAvrThermostat::GetScreenMemory() const {
  return Screen->GetDisplayMemory();
}
