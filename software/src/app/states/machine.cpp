#include "machine.hpp"

#include <ThermoSaveData_bp.h>
#include <driver_ds1307.h>
#include <etl/placement_new.h>

#include <driver_rs_wrapper.hpp>

#include "HomeAssistantSerial.hpp"
#include "coolable_parent.hpp"
#include "cooling.hpp"
#include "heating.hpp"
#include "idle.hpp"
#include "program_screen.hpp"
#include "state.hpp"

inline constexpr uint8_t u8(auto x) {
  return static_cast<uint8_t>(x);
}

// ===================================================================== //

void Machine::SetThermoSaveData(const ThermoSaveData& raw) {
  SaveData = raw;
}

void Machine::SetThermoButtonState(const ThermoButtonState& raw) {
  ButtonData = raw;
}

[[nodiscard]] const SafeThermoSaveData& Machine::SafeSaveState() const {
  return SaveData;
}

[[nodiscard]] SafeThermoSaveData& Machine::SafeSaveState() {
  return SaveData;
}

[[nodiscard]] ThermoButtonState& Machine::ButtonState() {
  return ButtonData;
}

void Machine::ResetStateChangeData() {
  ::new (&ChData) StateChangeData();
}

void Machine::ResetAutoTimeData() {
  ::new (&AtData) ProgramAutoTimeData();
}

ProgramAutoTimeData& Machine::AutoTimeData() {
  return AtData;
}

void Machine::TickChangeCounter() {
  if (ChData.StateChangeTimeoutSec < ChData.MaxStateChangeTimeoutSec) {
    ChData.StateChangeTimeoutSec += 1;
  }
}

void Machine::ReadTemperatureAndPeepIfChanged() {
  LastReadTemp = DriverReadTemp();

  if (LastReadTemp != LastCommTemp) {
    LastCommTemp = LastReadTemp;
    DriverDisplayTemp(LastCommTemp, SafeSaveState().TemperatureUnit());
    WriteHaTempStateTopicResponse();
  }
}

[[nodiscard]] bool Machine::HasChangeTimeoutPassed() const {
  return ChData.StateChangeTimeoutSec >= ChData.MaxStateChangeTimeoutSec;
}

[[nodiscard]] State::Type Machine::ChangeSetPoint(int8_t change) {
  auto& setPoint = SaveData.Data.set_point;

  if (setPoint == 1 && change < 0) {
    return State::Type::NO_CHANGE;
  }

  if (setPoint == 100 && change > 0) {
    return State::Type::NO_CHANGE;
  }

  setPoint += change;

  DriverDisplaySetPoint(setPoint, SaveData.TemperatureUnit());

  return DetermineNextState();
}

[[nodiscard]] State::Type Machine::DetermineNextState() {
  if (!HasChangeTimeoutPassed()) {
    return State::Type::NO_CHANGE;
  }

  const auto heatMode = SaveData.HeatMode();
  const auto setPoint = SaveData.Data.set_point;

  if (heatMode == HeatModeT::None) {
    return State::Type::Idle;
  }

  if (LastReadTemp >= setPoint && heatMode == HeatModeT::Heating) {
    return State::Type::Idle;
  } else if (LastReadTemp <= setPoint && heatMode == HeatModeT::Cooling) {
    return State::Type::Idle;
  } else if (LastReadTemp <= setPoint && heatMode == HeatModeT::Heating) {
    return State::Type::Heating;
  } else if (LastReadTemp >= setPoint && heatMode == HeatModeT::Cooling) {
    return State::Type::Cooling;
  }

  return State::Type::NO_CHANGE;
}

void Machine::ActivateCoolingRelays(Relay onRelay, Relay offRelay,
                                    ReverseValveModeT onIfType) {
  if (ButtonState().ReverseValveState == onIfType) {
    DriverRelayOn(Relay::ReversingValve);
  } else {
    DriverRelayOff(Relay::ReversingValve);
  }

  DriverRelayOn(onRelay);
  DriverRelayOff(offRelay);
}

void Machine::EnterHeatingOrCooling(HeatModeT mode) {
  if (SafeSaveState().FanMode() == FanModeT::Auto) {
    DriverRelayOn(Relay::Fan);
  }
}

void Machine::ExitHeatingOrCooling() {
  ResetStateChangeData();

  DriverRelayOff(Relay::Heat);
  DriverRelayOff(Relay::Compressor);
  DriverRelayOff(Relay::ReversingValve);

  if (SafeSaveState().FanMode() == FanModeT::Auto) {
    DriverRelayOff(Relay::Fan);
  }
}

bool Machine::IsHeatingOrCoolingNow() const {
  return get_state_id() == State::Type::Heating ||
         get_state_id() == State::Type::Cooling;
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

  if (SafeSaveState().FanMode() == FanModeT::On) {
    DriverRelayOn(Relay::Fan);
  }
}

void Machine::DisplayTemperature() {
  DriverDisplayTemp(LastCommTemp, SafeSaveState().TemperatureUnit());
}

void Machine::DisplaySetPointAndTemp() {
  const auto& save = SafeSaveState();
  DriverDisplaySetPoint(save.Data.set_point, (save.TemperatureUnit()));
  DriverDisplayTemp(LastCommTemp, SafeSaveState().TemperatureUnit());
}

void Machine::start() {
  ::new (CurrentState.get_address<void*>()) Idle(*this);

  ResetStateChangeData();

  ReadAndApplySettings();

  DisplaySetPointAndTemp();
}

void Machine::receive(const Event::Base& event) {
  auto newState = CurrentState.get_reference<State::Base>().handle_event(event);

  if (newState != get_state_id() && newState != State::Type::NO_CHANGE) {
    SwitchState(newState, event.id_);
  }
}

void Machine::receive(const HaCommand& c) {
  switch (static_cast<HaInTopicKey>(c.topic_key)) {
    case HaInTopicKey::FanModeCommandTopic:
      SafeSaveState().Data.fan_mode = c.payload_byte_one;
      WriteHaFanModeTopicResponse();
      // TODO: Update Reality
      break;
    case HaInTopicKey::ModeCommandTopic:
      SafeSaveState().Data.heat_mode = c.payload_byte_one;
      WriteHaModeStateTopicResponse();
      // TODO: Update Reality
      break;
    case HaInTopicKey::PowerCommandTopic:
      break;
    case HaInTopicKey::PresetModeCommandTopic:
      break;
    case HaInTopicKey::TempCommandTopic:
      SafeSaveState().Data.set_point = c.payload_byte_one;
      WriteHaTempStateTopicResponse();
      // TODO: Update Reality
      break;
    case HaInTopicKey::TempHighCommandTopic:
      SafeSaveState().Data.auto_high_set_point = c.payload_byte_one;
      WriteHaSerialResponse(HaOutTopicKey::TempHighStateTopic,
                            c.payload_byte_one, 0);
      // TODO: Update Reality
      break;
    case HaInTopicKey::TempLowCommandTopic:
      SafeSaveState().Data.auto_low_set_point = c.payload_byte_one;
      WriteHaSerialResponse(HaOutTopicKey::TempLowStateTopic,
                            c.payload_byte_one, 0);
      // TODO: Update Reality
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
      ::new (address) TempScreen(SaveData.Data, lastEventWasDown);
      break;
    case State::Type::ProgramDate:
      ::new (address) DateScreen(SaveData.Data, lastEventWasDown);
      break;
    case State::Type::ProgramTime:
      ::new (address) TimeScreen(SaveData.Data, lastEventWasDown);
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
  const auto& saveData = SaveData.Data;
  DriverSaveData(saveData);
  DriverSetTimeFromSaveData(saveData.time, saveData.date);
}

void Machine::WriteHaTempStateTopicResponse() const {
  WriteHaSerialResponse(HaOutTopicKey::TempStateTopic, LastReadTemp, 0);
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
                        SafeSaveState().Data.heat_mode, 0);
}

void Machine::WriteHaFanModeTopicResponse() const {
  WriteHaSerialResponse(HaOutTopicKey::FanModeStateTopic,
                        SafeSaveState().Data.fan_mode, 0);
}

void Machine::WriteHaSerialResponse(HaOutTopicKey topic, uint8_t byte_one,
                                    uint8_t byte_two) const {
  uint8_t topic_u8 = u8(topic);
  HaCommand c;
  c.topic_key = topic_u8;
  c.payload_byte_one = byte_one;
  c.payload_byte_two = byte_two;
  c.checksum = topic_u8 + byte_one + byte_two;
  uint8_t b[BYTES_LENGTH_HA_COMMAND];
  EncodeHaCommand(&c, b);
  DriverWriteSerialPortRaw(b, sizeof(b));
}

// ===================================================================== //

SafeThermoSaveData::SafeThermoSaveData() {
  Data.magic = THERMO_STATE_DATA_MAGIC;

  // Good defaults
  Data.set_point = 77;
  Data.temp_display_unit = TEMP_UNIT_FREEDOM;
  Data.fan_mode = FANMODE_AUTO;
  Data.heat_mode = HEATMODE_NONE;

  // Default date of 2000-01-01
  Data.date.day = 1;
  Data.date.month = 1;
  Data.date.year = 0;
  Data.date.day_of_week = DAYOFWEEK_SATURDAY;  // Saturday

  // Default time of 12:00 AM
  Data.time.second = 0;
  Data.time.minute = 0;
  Data.time.hour = 0;
  Data.time.am_pm = TIME_AM;
}

TemperatureUnitT SafeThermoSaveData::TemperatureUnit() const {
  return static_cast<TemperatureUnitT>(Data.temp_display_unit);
}

HeatModeT SafeThermoSaveData::HeatMode() const {
  return static_cast<HeatModeT>(Data.heat_mode);
}

FanModeT SafeThermoSaveData::FanMode() const {
  return static_cast<FanModeT>(Data.fan_mode);
}

SafeThermoSaveData::SafeThermoSaveData(const ThermoSaveData& other) {
  Data = other;
}

HeatModeT SafeThermoSaveData::BumpHeatingMode() {
  switch (static_cast<HeatModeT>(Data.heat_mode)) {
    case HeatModeT::Heating:
      Data.heat_mode = u8(HeatModeT::Cooling);
      break;
    case HeatModeT::Cooling:
      Data.heat_mode = u8(HeatModeT::None);
      break;
    case HeatModeT::None:
      Data.heat_mode = u8(HeatModeT::Heating);
      break;
  }
  return static_cast<HeatModeT>(Data.heat_mode);
}

FanModeT SafeThermoSaveData::BumpFanMode() {
  switch (static_cast<FanModeT>(Data.fan_mode)) {
    case FanModeT::On:
      Data.fan_mode = u8(FanModeT::Auto);
      break;
    case FanModeT::Auto:
      Data.fan_mode = u8(FanModeT::On);
      break;
  }
  return static_cast<FanModeT>(Data.fan_mode);
}
