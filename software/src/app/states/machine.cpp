#include "machine.hpp"

#include <ThermoSaveData_bp.h>
#include <driver_ds1307.h>
#include <etl/algorithm.h>
#include <etl/placement_new.h>

#include <driver_rs_wrapper.hpp>

#include "HomeAssistantSerial.hpp"
#include "casts.hpp"
#include "checksum.hpp"
#include "coolable_parent.hpp"
#include "cooling.hpp"
#include "event.hpp"
#include "heating.hpp"
#include "idle.hpp"
#include "program_screen.hpp"
#include "safe_thermo_safe.hpp"
#include "state.hpp"
#include "states/started.hpp"
#include "temperature.hpp"

// ===================================================================== //
//

void Machine::start() {
  ::new (CurrentState.get_address<void*>()) Started();
  ReadTemperature();
  ReadAndApplySettings();
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
  ::new (&ProgData) ProgramData();
}

ProgramData& Machine::AutoTimeData() {
  return ProgData;
}

void Machine::ReadTemperature() {
  uint16_t temperatureTicks = 0, humidityTicks = 0;
  DriverReadRawTemp(temperatureTicks, humidityTicks);
  CurrentTemp.SetFromSht4xSensor(temperatureTicks);
  CurrentHumidity.SetFromSht4xSensor(humidityTicks);
}

void Machine::ReadTemperatureAndReportIfChanged() {
  ReadTemperature();

  const bool TempChanged =
    CurrentTemp.GetUnitWhole(TemperatureUnitT::Celsius) !=
    PreviousTemp.GetUnitWhole(TemperatureUnitT::Celsius);

  const bool HumidChanged =
    CurrentHumidity.ToPercent() != PreviousHumidity.ToPercent();

  if (TempChanged || HumidChanged) {
    DisplayTemperature();

    if (TempChanged) {
      PreviousTemp = CurrentTemp;
      WriteHaTempStateTopicResponse();
    }

    if (HumidChanged) {
      PreviousHumidity = CurrentHumidity;
      WriteHaHumidityStateTopicResponse();
    }
  }
}

const Temperature& Machine::CurrentTemperature() const {
  return CurrentTemp;
}

void Machine::ReadAndApplySettings() {
  const bool loadSuccess = DriverLoadData(SaveData.MutableRaw());

  // If something went wrong, save data is now corrupted. Reset it.
  if (!loadSuccess) {
    ::new (&SaveData) SafeThermoSaveData();
  }

  ThermoButtonState buttons;
  DriverGetButtonStateNow(&buttons);
  SetThermoButtonState(buttons);

  ApplySaveState();
}

void Machine::DisplayTemperature() {
  DriverDisplayTemp(CurrentTemp, CurrentHumidity,
                    SaveState().TemperatureUnit());
}

void Machine::DisplaySetPointAndTemp() {
  const auto& save = SaveState();
  DriverDisplaySetPoint(save.SetPoint(), (save.TemperatureUnit()));
  DriverDisplayTemp(CurrentTemp, CurrentHumidity,
                    SaveState().TemperatureUnit());
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
      ApplySaveState();
      break;
    case HaInTopicKey::ModeCommandTopic:
      SaveState().HeatMode() = static_cast<HeatModeT>(c.payload_byte_one);
      WriteHaModeStateTopicResponse();
      ApplySaveState();
      break;
    case HaInTopicKey::PowerCommandTopic:
      break;
    case HaInTopicKey::PresetModeCommandTopic:
      break;
    case HaInTopicKey::TempCommandTopic:
      SaveData.SetSetPoint(Temperature::FromCelcius(c.payload_byte_one));
      break;
    case HaInTopicKey::MqttPing:
      MqttData.ResetTimeout();
      break;
  }
}

State::Type Machine::get_state_id() const {
  return CurrentState.get_reference<State::Base>().StateId;
}

bool IsProgrammingState(State::Type s) {
  return s == State::Type::ProgramTemp || s == State::Type::ProgramDate ||
         s == State::Type::ProgramTime;
}

void Machine::SwitchState(State::Type new_state, Event::Type lastEvent) {
  State::Base* address = CurrentState.get_address<State::Base>();

  const auto prevState = address->StateId;

  const bool enteringProgramming =
    !IsProgrammingState(prevState) && IsProgrammingState(new_state);

  const bool exitingProgramming =
    IsProgrammingState(prevState) && !IsProgrammingState(new_state);

  if (enteringProgramming) {
    SetupProgramming();
  } else if (exitingProgramming) {
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
      ::new (address) TempScreen(SaveData.MutableRaw(), lastEventWasDown);
      break;
    case State::Type::ProgramDate:
      ::new (address) DateScreen(ProgData.ChangedTime_, lastEventWasDown);
      break;
    case State::Type::ProgramTime:
      ::new (address) TimeScreen(ProgData.ChangedTime_, lastEventWasDown);
      break;
    case State::Type::Started:    // Should never happen
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

void Machine::SetupProgramming() {
  ::new (&ProgData) ProgramData();
  DriverGetTime(ProgData.StartTime_);
  memcpy(&ProgData.ChangedTime_, &ProgData.StartTime_, sizeof(ds1307_time_s));
}

void Machine::SaveProgrammingSettings() {
  const auto& saveData = SaveData.Raw();
  DriverSaveData(saveData);
  if (ProgData.WasTimeChanged()) {
    DriverSetTime(ProgData.ChangedTime_);
  }
}

void Machine::ApplySaveState() {
  if (SaveData.FanMode() == FanModeT::On) {
    DriverRelayOn(Relay::Fan);
  } else {
    DriverRelayOff(Relay::Fan);
  }

  switch (SaveData.HeatMode()) {
    case HeatModeT::Heating:
      SwitchState(State::Type::Heating, Event::Type::SecondPassed);
      break;
    case HeatModeT::Cooling:
      SwitchState(State::Type::Cooling, Event::Type::SecondPassed);
      break;
    case HeatModeT::None:
      SwitchState(State::Type::Idle, Event::Type::SecondPassed);
      break;
  }
}

void Machine::WriteHaTempStateTopicResponse() const {
  WriteHaSerialResponse(HaOutTopicKey::TempStateTopic,
                        CurrentTemp.GetUnitWhole(TemperatureUnitT::Celsius), 0);
}

void Machine::WriteHaHumidityStateTopicResponse() const {
  WriteHaSerialResponse(HaOutTopicKey::HumidityStateTopic,
                        CurrentHumidity.ToPercent(), 0);
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
  HaCommand c;
  c.checksum = 0;
  c.topic_key = u8(topic);
  c.payload_byte_one = byte_one;
  c.payload_byte_two = byte_two;
  uint8_t b[BYTES_LENGTH_HA_COMMAND];  // 0 0 0 0
  memset(b, 0, sizeof(b));
  EncodeHaCommand(&c, b);
  b[0] = checksum(b + 1, sizeof(b) - 1);
  DriverWriteSerialPortRaw(b, sizeof(b));
}
