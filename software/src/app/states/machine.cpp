#include "machine.hpp"

#include <Noritake_VFD_GU7000.h>
#include <ThermoSaveData_bp.h>
#include <driver_ds1307.h>
#include <etl/placement_new.h>

#include <HomeAssistantSerial.hpp>
#include <casts.hpp>
#include <checksum.hpp>
#include <driver_rs_wrapper.hpp>
#include <temperature.hpp>

#include "event.hpp"
#include "safe_thermo_safe.hpp"
#include "state.hpp"
#include "states/started.hpp"

// ===================================================================== //

void Machine::start() {
  ::new (CurrentState.get_address<void*>()) Started();

  ReadTemperature();
  ReadAndApplySettings();

  PreviousTemp = CurrentTemp;
  PreviousHumidity = CurrentHumid;

  {
    AutoTwi t;
    DriverGetScreenHandle().GU7000_setScreenBrightness(1);
  }

  InitialRender();
}

[[nodiscard]] const SafeThermoSaveData& Machine::SaveState() const {
  return SaveData;
}

[[nodiscard]] SafeThermoSaveData& Machine::SaveState() {
  return SaveData;
}

ProgramData& Machine::AutoTimeData() {
  return ProgData;
}

void Machine::ReadTemperature() {
  uint16_t temperatureTicks = 0, humidityTicks = 0;
  DriverReadRawTemp(temperatureTicks, humidityTicks);
  CurrentTemp.SetFromSht4xSensor(temperatureTicks);
  CurrentHumid.SetFromSht4xSensor(humidityTicks);
}

void Machine::ReadTemperatureAndReportIfChanged() {
  ReadTemperature();

  const auto unit = SaveData.TemperatureUnit();
  const auto prevTemp = PreviousTemp.GetRoundedUnitWhole(unit);
  const auto nowTemp = CurrentTemp.GetRoundedUnitWhole(unit);

  if (nowTemp != prevTemp) {
    rctx_.temperature_manager_.Calculate(prevTemp, nowTemp);
    WriteHaCurrentTopicResponse();
    PreviousTemp = CurrentTemp;
  }

  const auto prevHumid = PreviousHumidity.ToWholePercent();
  const auto nowHumid = CurrentHumid.ToWholePercent();

  if (nowHumid != prevHumid) {
    WriteHaCurrentHumidityResponse();
    rctx_.humidity_manager_.Calculate(prevHumid, nowHumid);
    PreviousHumidity = CurrentHumid;
  }
}

const Temperature& Machine::CurrentTemperature() const {
  return CurrentTemp;
}

const Humidity& Machine::CurrentHumidity() const {
  return CurrentHumid;
}

void Machine::ReadAndApplySettings() {
  const bool loadSuccess = DriverLoadData(SaveData.MutableRaw());

  // If something went wrong, save data is now corrupted. Reset it.
  // TODO: Setup screen if nothing is there
  if (!loadSuccess) {
    ::new (&SaveData) SafeThermoSaveData();
  }

  ApplySaveState();
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
    case HaInTopicKey::TempCommandTopic:
      SaveData.SetSetPoint(Temperature::FromCelcius(c.payload_byte_one));
      break;
  }
}

State::Type Machine::get_state_id() const {
  return CurrentState.get_reference<State::Base>().StateId;
}

void Machine::SwitchState(State::Type new_state, Event::Type lastEvent) {
  State::Base* address = CurrentState.get_address<State::Base>();

  const bool lastEventWasDown = lastEvent == Event::Type::DownButtonPressed;

  HandleProgrammingTransition(address->StateId, new_state);

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
      ::new (address) DateScreen(ProgData.changed_time, lastEventWasDown);
      break;
    case State::Type::ProgramTime:
      ::new (address) TimeScreen(ProgData.changed_time, lastEventWasDown);
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

bool IsProgrammingState(State::Type s) {
  return s == State::Type::ProgramTemp || s == State::Type::ProgramDate ||
         s == State::Type::ProgramTime;
}

void Machine::HandleProgrammingTransition(State::Type prevState,
                                          State::Type new_state) {
  const bool wasProgramming = IsProgrammingState(prevState);
  const bool isNewProgramming = IsProgrammingState(new_state);

  const bool enteringProgramming = !wasProgramming && isNewProgramming;
  const bool exitingProgramming = wasProgramming && !isNewProgramming;

  if (enteringProgramming) {
    SetupProgramming();
  } else if (exitingProgramming) {
    SaveProgrammingSettings();
    InitialRender();
  }
}

void Machine::InitialRender() {
  const auto unit = SaveData.TemperatureUnit();
  const auto currentTemp = CurrentTemp.GetRoundedUnitWhole(unit);
  const auto currentHumid = CurrentHumid.ToWholePercent();
  const auto setPoint = SaveData.SetPoint().GetRoundedUnitWhole(unit);

  DriverDisplayClearScreen();

  rctx_.renderer_.InitializeDigitImages(currentTemp, currentHumid);
  rctx_.renderer_.DrawTemperature(unit);
  rctx_.renderer_.DrawHumidity();
  rctx_.renderer_.DrawHeatingStatus(SaveData.HeatMode(), false);
  rctx_.renderer_.DrawSetPoint(setPoint);
  rctx_.temperature_manager_.Calculate(currentTemp, currentTemp);
  rctx_.humidity_manager_.Calculate(currentHumid, currentHumid);
}

void Machine::SetupProgramming() {
  ::new (&ProgData) ProgramData();

  ProgData.original_temp_unit = SaveData.TemperatureUnit();
  DriverGetTime(ProgData.original_time);
  memcpy(&ProgData.changed_time, &ProgData.original_time,
         sizeof(ds1307_time_s));
}

void Machine::SaveProgrammingSettings() {
  const auto nowSetpoint = SaveData.SetPoint();
  const auto& nowUnit = SaveData.TemperatureUnit();

  const bool cToF = ProgData.original_temp_unit == TemperatureUnitT::Celsius &&
                    nowUnit == TemperatureUnitT::Freedom;

  const bool fToC = ProgData.original_temp_unit == TemperatureUnitT::Freedom &&
                    nowUnit == TemperatureUnitT::Celsius;

  if (cToF) {
    auto t = Temperature::FromFahrenheit(
        nowSetpoint.GetRoundedUnitWhole(TemperatureUnitT::Freedom));
    SaveData.SetSetPoint(t);
  } else if (fToC) {
    auto t = Temperature::FromCelcius(
        nowSetpoint.GetRoundedUnitWhole(TemperatureUnitT::Celsius));
    SaveData.SetSetPoint(t);
  }

  DriverSaveData(SaveData.Raw());

  if (ProgData.WasTimeChanged()) {
    DriverSetTime(ProgData.changed_time);
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

void Machine::WriteHaCurrentTopicResponse() const {
  WriteHaSerialResponse(
      HaOutTopicKey::CurrentTempTopic,
      CurrentTemp.GetRoundedUnitWhole(TemperatureUnitT::Celsius), 0);
}

void Machine::WriteHaCurrentHumidityResponse() const {
  WriteHaSerialResponse(HaOutTopicKey::CurrentHumidityTopic,
                        CurrentHumid.ToWholePercent(), 0);
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

void Machine::WriteHaSerialResponse(HaOutTopicKey topic,
                                    uint8_t byte_one,
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

// ===================================================================== //

RenderContext::RenderContext()
    : renderer_(DriverGetScreenHandle(), images_),
      temperature_manager_(images_.temperature_hundreds_or_minus_,
                           images_.temperature_tens_,
                           images_.temperature_ones_),
      humidity_manager_(dummy_humidity_hundreds_,
                        images_.humidity_tens_,
                        images_.humidity_ones_) {}

// ===================================================================== //

void ProgramData::Init(SafeThermoSaveData& data) {
  original_temp_unit = data.TemperatureUnit();
  DriverGetTime(original_time);
  memcpy(&changed_time, &original_time, sizeof(ds1307_time_s));
}

bool ProgramData::WasTimeChanged() const {
  return memcmp(&original_time, &changed_time, sizeof(original_time)) != 0;
}
