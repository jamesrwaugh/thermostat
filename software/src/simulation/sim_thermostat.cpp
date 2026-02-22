#include "sim_thermostat.hpp"

#include <simavr/avr_uart.h>
#include <simavr/sim_irq.h>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <simavr-toolbox/sim_base.hpp>
#include <simavr-toolbox/sim_bouncy_switch.hpp>
#include <simavr-toolbox/sim_gu7000.hpp>

#include "HomeAssistantSerial.hpp"
#include "HomeAssistantSerial_bp.h"
#include "thermo_data_types.hpp"

void FakeCb(struct avr_irq_t* irq, uint32_t value, void* param) {
  auto cb = (SimAvrThermostat::RelayCb*)param;
  bool on = value != 0;

  if (strcmp(irq->name, "=avr.portc.pin0") == 0) {
    (*cb)(RelayType::Fan, on);
  } else if (strcmp(irq->name, "=avr.portb.pin2") == 0) {
    (*cb)(RelayType::Compressor, on);
  } else if (strcmp(irq->name, "=avr.portc.pin1") == 0) {
    (*cb)(RelayType::Heat, on);
  } else if (strcmp(irq->name, "=avr.portc.pin2") == 0) {
    (*cb)(RelayType::ReverseValve, on);
  } else {
    std::abort();
  }
}

SimAvrThermostat::SimAvrThermostat(std::string_view filename,
                                   bool gdb,
                                   TaskReceiver& receiver)
    : FtxUiSimulatedAvr(filename, gdb, receiver) {
  // Screen
  Screen = std::make_unique<SimFtdiGu7000>(Avr_);

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

  // Temperature
  Temp_ = std::make_unique<SimSHT4x>(Avr_);

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

void SimAvrThermostat::OnRelayChange(RelayType r, bool value) {
  switch (r) {
    case RelayType::Fan:
      Relays_.Fan = value;
      break;
    case RelayType::Compressor:
      Relays_.Compressor = value;
      break;
    case RelayType::Heat:
      Relays_.Heat = value;
      break;
    case RelayType::ReverseValve:
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
    // Screen->OnMillisecondPassed();
  }

  Temp_->SimulateTempChange(Relays_);
}

const char* FormatAction(uint8_t key) {
  switch (key) {
    case HA_ACTION_OFF:
      return "Off";
    case HA_ACTION_HEATING:
      return "Heating";
    case HA_ACTION_COOLING:
      return "Cooling";
    case HA_ACTION_IDLE:
      return "Idle";
    case HA_ACTION_FAN:
      return "Fan";
    default:
      return "?";
  }
}

const char* FormatFanMode(FanModeT key) {
  switch (key) {
    case FanModeT::On:
      return "On";
    case FanModeT::Auto:
      return "Auto";
    default:
      return "?";
  }
}

const char* FormatHeatMode(HeatModeT key) {
  switch (key) {
    case HeatModeT::Heating:
      return "Heating";
    case HeatModeT::Cooling:
      return "Cooling";
    case HeatModeT::None:
      return "None";
    default:
      return "?";
  }
}

std::string FormatHaCommand(const HaCommand& c) {
  switch (static_cast<HaOutTopicKey>(c.topic_key)) {
    case HaOutTopicKey::ActionTopic:
      return std::format("[Action] {}", FormatAction(c.payload_byte_one));
    case HaOutTopicKey::CurrentHumidityTopic:
      return std::format("[CurrentHumid] {}%", c.payload_byte_one);
    case HaOutTopicKey::CurrentTempTopic:
      return std::format("[CurrentTemp] {}C", c.payload_byte_one);
    case HaOutTopicKey::FanModeStateTopic:
      return std::format(
          "[FanMode] {}",
          FormatFanMode(static_cast<FanModeT>(c.payload_byte_one)));
    case HaOutTopicKey::ModeStateTopic:
      return std::format(
          "[ModeState] {}",
          FormatHeatMode(static_cast<HeatModeT>(c.payload_byte_one)));
    case HaOutTopicKey::TempStateTopic:
      return std::format("[TempState] {}C", c.payload_byte_one);
    default:
      return "?";
  }
}

void SimAvrThermostat::OnUartByteReceived(int uartNumber, uint8_t byte) {
  if (uartNumber == 0) {
    HaCommand c;
    c.checksum = 0;
    if (mail.ReceiveByte(byte, c)) {
      sim_debug_log(FormatHaCommand(c));
    } else if (c.checksum == 0xFF) {
      sim_debug_log("Bad [%d] %d %d", c.topic_key, c.payload_byte_one,
                    c.payload_byte_two);
    }
  }
}

const SimGu7000::DisplayMemory& SimAvrThermostat::GetScreenMemory() const {
  static SimGu7000::DisplayMemory s;
  return s;
}
