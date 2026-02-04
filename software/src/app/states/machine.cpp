#include "machine.hpp"

#include <ThermoSaveData_bp.h>
#include <driver_ds1307.h>
#include <etl/placement_new.h>

#include <driver_rs_wrapper.hpp>

#include "HomeAssistantSerial.hpp"
#include "casts.hpp"
#include "coolable_parent.hpp"
#include "cooling.hpp"
#include "event.hpp"
#include "heating.hpp"
#include "idle.hpp"
#include "program_screen.hpp"
#include "state.hpp"

// ===================================================================== //

void Machine::SetThermoSaveData(const ThermoSaveData& raw) {
  SaveData = raw;
}

void Machine::SetThermoButtonState(const ThermoButtonState& raw) {
  ButtonData = raw;
}

[[nodiscard]] const SafeThermoSaveData& Machine::SaveState() const {
  return SaveData;
}

[[nodiscard]] SafeThermoSaveData& Machine::SaveState() {
  return SaveData;
}

[[nodiscard]] ThermoButtonState& Machine::ButtonState() {
  return ButtonData;
}

void Machine::ResetAutoTimeData() {
  ::new (&AtData) ProgramAutoTimeData();
}

ProgramAutoTimeData& Machine::AutoTimeData() {
  return AtData;
}

void Machine::ReadTemperatureAndReportIfChanged() {
  LastReadTemp = DriverReadTemp();

  if (LastReadTemp != LastCommTemp) {
    LastCommTemp = LastReadTemp;
    DriverDisplayTemp(LastCommTemp, SaveState().TemperatureUnit());
    WriteHaTempStateTopicResponse(LastCommTemp);
  }
}

uint8_t Machine::LastReadTemerature() const {
  return LastReadTemp;
}

void Machine::ReadAndApplySettings() {
  ThermoSaveData data;
  const bool loadSuccess = DriverLoadData(data);

  if (loadSuccess) {
    SetThermoSaveData(data);
  }

  ThermoButtonState buttons;
  DriverGetButtonStateNow(&buttons);
  SetThermoButtonState(buttons);

  if (SaveState().FanMode() == FanModeT::On) {
    DriverRelayOn(Relay::Fan);
  }
}

void Machine::DisplayTemperature() {
  DriverDisplayTemp(LastCommTemp, SaveState().TemperatureUnit());
}

void Machine::DisplaySetPointAndTemp() {
  const auto& save = SaveState();
  DriverDisplaySetPoint(save.SetPoint(), (save.TemperatureUnit()));
  DriverDisplayTemp(LastCommTemp, SaveState().TemperatureUnit());
}

void Machine::start() {
  ::new (CurrentState.get_address<void*>()) Idle(*this);
  ReadAndApplySettings();
  DisplaySetPointAndTemp();
}

// Setting: MQTT Config

void Machine::receive(const Event::Base& event) {
  switch (event.id_) {
    case Event::Type::SecondPassed:
      MqttData.IncreaseTimeout();
      if (MqttData.IsMqttConnected()) {
      }
      break;
    default:
      break;
  }

  auto newState = CurrentState.get_reference<State::Base>().handle_event(event);

  if (newState != get_state_id() && newState != State::Type::NO_CHANGE) {
    SwitchState(newState, event.id_);
  }
}

void Machine::receive(const HaCommand& c) {
  switch (static_cast<HaInTopicKey>(c.topic_key)) {
    case HaInTopicKey::FanModeCommandTopic:
      SaveState().FanMode() = static_cast<FanModeT>(c.payload_byte_one);
      WriteHaFanModeTopicResponse();
      // TODO: Update Reality
      break;
    case HaInTopicKey::ModeCommandTopic:
      SaveState().HeatMode() = static_cast<HeatModeT>(c.payload_byte_one);
      WriteHaModeStateTopicResponse();
      // TODO: Update Reality
      break;
    case HaInTopicKey::PowerCommandTopic:
      break;
    case HaInTopicKey::PresetModeCommandTopic:
      break;
    case HaInTopicKey::TempCommandTopic:
      SaveState().SetPoint() = c.payload_byte_one;
      // TODO: Update Reality
      break;
    case HaInTopicKey::MqttPing:
      MqttData.ResetTimeout();
      break;
  }
}

State::Type Machine::get_state_id() const {
  return CurrentState.get_reference<State::Base>().StateId;
}

void Machine::SwitchState(State::Type new_state, Event::Type lastEvent) {
  State::Base* address = CurrentState.get_address<State::Base>();

  const auto prevState = address->StateId;

  const bool wasProgramming = prevState == State::Type::ProgramTemp ||
                              prevState == State::Type::ProgramDate ||
                              prevState == State::Type::ProgramTime;

  if (wasProgramming && new_state == State::Type::Idle) {
    SaveProgrammingSettings();
  }

  const bool lastEventWasDown = lastEvent == Event::Type::DownButtonPressed;

  address->~Base();

  switch (new_state) {
    case State::Type::Idle:
      ::new (address) Idle(*this);
      break;
    case State::Type::Heating:
      ::new (address) Heating(*this);
      break;
    case State::Type::Cooling:
      ::new (address) Cooling(*this);
      break;
    case State::Type::ProgramTemp:
      ::new (address) TempScreen(SaveData.RawRaw(), lastEventWasDown);
      break;
    case State::Type::ProgramDate:
      ::new (address) DateScreen(SaveData.RawRaw(), lastEventWasDown);
      break;
    case State::Type::ProgramTime:
      ::new (address) TimeScreen(SaveData.RawRaw(), lastEventWasDown);
      break;
    case State::Type::NO_CHANGE:  // Should never happen
      ::new (address) Idle(*this);
      break;
    case State::Type::ProgramAutoTimeSelect:
    case State::Type::ProgramAutoTimeDates:
    case State::Type::ProgramAutoTimeStart:
    case State::Type::ProgramAutoTimeEnd:
    case State::Type::ProgramAutoTimeTemps:
      break;
  }
}

void Machine::SaveProgrammingSettings() {
  const auto& saveData = SaveData.Raw();
  DriverSaveData(saveData);
  DriverSetTimeFromSaveData(saveData.time, saveData.date);
}

void Machine::WriteHaTempStateTopicResponse(uint8_t temp) const {
  WriteHaSerialResponse(HaOutTopicKey::TempStateTopic, temp, 0);
}

void Machine::WriteHaActionStateTopicResponse(HaActionKey key) const {
  // What we are doing now
  // off, heating, cooling, drying, idle, fan
  WriteHaSerialResponse(HaOutTopicKey::ActionTopic, u8(key), 0);
}

void Machine::WriteHaModeStateTopicResponse() const {
  // The goal setting of the system
  // cooling, heating, none
  WriteHaSerialResponse(HaOutTopicKey::ModeStateTopic,
                        u8(SaveState().HeatMode()), 0);
}

void Machine::WriteHaFanModeTopicResponse() const {
  WriteHaSerialResponse(HaOutTopicKey::FanModeStateTopic,
                        u8(SaveState().FanMode()), 0);
}

void Machine::WriteHaSerialResponse(HaOutTopicKey topic, uint8_t byte_one,
                                    uint8_t byte_two) const {
  const uint8_t topic_u8 = u8(topic);
  const uint16_t checksum = topic_u8 + byte_one + byte_two;
  HaCommand c;
  c.topic_key = topic_u8;
  c.payload_byte_one = byte_one;
  c.payload_byte_two = byte_two;
  c.checksum = checksum & 0xFF;
  uint8_t b[BYTES_LENGTH_HA_COMMAND];
  EncodeHaCommand(&c, b);
  DriverWriteSerialPortRaw(b, sizeof(b));
}
