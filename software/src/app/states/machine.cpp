#include "machine.hpp"

#include <etl/placement_new.h>

#include <driver_rs_wrapper.hpp>

#include "coolable_parent.hpp"
#include "cooling.hpp"
#include "heating.hpp"
#include "idle.hpp"
#include "program.hpp"
#include "protos/ThermoCommEvent_bp.h"
#include "protos/ThermoSaveData_bp.h"
#include "state.hpp"

TemperatureUnitT SafeThermoSaveData::TemperatureUnit() const {
  switch (Data.temp_display_unit) {
    case TEMP_UNIT_FREEDOM:
      return TemperatureUnitT::Freedom;
    case TEMP_UNIT_CELSIUS:
      return TemperatureUnitT::Celsius;
    default:
      return TemperatureUnitT::Freedom;
  }
}

Machine::Machine() : LastReadTemp(0), LastCommTemp(0) {}

void Machine::SetThermoSaveData(const ThermoSaveData& raw) {
  SaveData = raw;
}

void Machine::SetThermoButtonState(const ThermoButtonState& raw) {
  ButtonData = raw;
}

[[nodiscard]] ThermoSaveData& Machine::SaveState() {
  return SaveData.Data;
}

[[nodiscard]] const ThermoSaveData& Machine::SaveState() const {
  return SaveData.Data;
}

[[nodiscard]] const SafeThermoSaveData& Machine::SafeSaveState() const {
  return SaveData;
}

[[nodiscard]] ThermoButtonState& Machine::ButtonState() {
  return ButtonData;
}

void Machine::ResetStateChangeData() {
  ::new (&ChData) StateChangeData();
}

void Machine::TickChangeCounter() {
  if (ChData.StateChangeTimeoutSec < ChData.MaxStateChangeTimeoutSec) {
    ChData.StateChangeTimeoutSec += 1;
  }
}

void Machine::ReadTemperature() {
  LastReadTemp = DriverReadTemp();

  if (LastReadTemp != LastCommTemp) {
    auto v = TempChangedEvent{.new_temp_f = LastReadTemp};
    Comms()(v);
    DriverDisplayTemp(LastReadTemp, SaveData.TemperatureUnit());
    LastCommTemp = LastReadTemp;
  }
}

[[nodiscard]] bool Machine::HasChangeTimeoutPassed() const {
  return ChData.StateChangeTimeoutSec >= ChData.MaxStateChangeTimeoutSec;
}

[[nodiscard]] State::Type Machine::ChangeSetPoint(int8_t change) {
  auto& setPoint = SaveData.Data.set_point;

  if (setPoint == 1 && change < 0) {
    return State::Type::NO_CHANGE;  // Stay in current state
  }

  if (setPoint == 100 && change > 0) {
    return State::Type::NO_CHANGE;  // Stay in current state
  }

  setPoint += change;

  DriverDisplaySetPoint(setPoint, SaveData.TemperatureUnit());

  auto v = SetPointChangedEvent{.new_set_point_f = setPoint};
  Comms()(v);

  return DetermineNextState();
}

[[nodiscard]] State::Type Machine::DetermineNextState() {
  const auto& buttons = ButtonState();

  if (!HasChangeTimeoutPassed()) {
    return State::Type::NO_CHANGE;
  }

  const auto heatMode = buttons.HeatingState;
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
  uint8_t mode2 = HEATING_COMM_IDLE;

  switch (mode) {
    case HeatModeT::Heating:
      mode2 = HEATING_COMM_HEATING;
      break;
    case HeatModeT::Cooling:
      mode2 = HEATING_COMM_COOLING;
      break;
    case HeatModeT::None:
      mode2 = HEATING_COMM_IDLE;
      break;
  }

  auto v = HeatingModeChangedEvent{.new_mode = mode2};
  Comms()(v);

  if (ButtonState().FanState == FanModeT::Auto) {
    DriverRelayOn(Relay::Fan);
  }
}

void Machine::ExitHeatingOrCooling() {
  ResetStateChangeData();

  DriverRelayOff(Relay::Heat);
  DriverRelayOff(Relay::Compressor);
  DriverRelayOff(Relay::ReversingValve);

  if (ButtonState().FanState == FanModeT::Auto) {
    DriverRelayOff(Relay::Fan);
  }
}

bool Machine::IsHeatingOrCoolingNow() const {
  return get_state_id() == State::Type::Heating ||
         get_state_id() == State::Type::Cooling;
}

SafeThermoSaveData::SafeThermoSaveData() {
  Data.magic = THERMO_STATE_DATA_MAGIC;
  Data.set_point = 70;
  Data.temp_display_unit = TEMP_UNIT_FREEDOM;
}

SafeThermoSaveData::SafeThermoSaveData(const ThermoSaveData& other) {
  Data = other;
}

static constexpr uint16_t PrinterId = 0x5645;

void Print(uint8_t commandId, uint8_t* buffer, uint8_t len) {
  DriverWriteSerialPortRawCh((PrinterId >> 8) & 0xFF);
  DriverWriteSerialPortRawCh(PrinterId & 0xFF);
  DriverWriteSerialPortRawCh(commandId);
  DriverWriteSerialPortRaw(buffer, len);
  DriverWriteSerialPortRawCh('\r');
  DriverWriteSerialPortRawCh('\n');
}

void SerialPrintVisitor::operator()(TempChangedEvent& e) {
  uint8_t b[BYTES_LENGTH_TEMP_CHANGED_EVENT];
  EncodeTempChangedEvent(&e, b);
  Print(0x00, b, sizeof(b));
}

void SerialPrintVisitor::operator()(SetPointChangedEvent& e) {
  uint8_t b[BYTES_LENGTH_SET_POINT_CHANGED_EVENT];
  EncodeSetPointChangedEvent(&e, b);
  Print(0x01, b, sizeof(b));
}

void SerialPrintVisitor::operator()(HeatingModeChangedEvent& e) {
  uint8_t b[BYTES_LENGTH_HEATING_MODE_CHANGED_EVENT];
  EncodeHeatingModeChangedEvent(&e, b);
  Print(0x02, b, sizeof(b));
}

void SerialPrintVisitor::operator()(ThermoSaveData& e) {
  uint8_t b[BYTES_LENGTH_THERMO_SAVE_DATA];
  EncodeThermoSaveData(&e, b);
  Print(0x03, b, sizeof(b));
}

SerialPrintVisitor& Machine::Comms() {
  return V;
}

void Machine::ReadAndApplySettings() {
  uint8_t rtcDataBuf[BYTES_LENGTH_THERMO_SAVE_DATA];
  DriverReadFlash(0, rtcDataBuf, sizeof(rtcDataBuf));

  ThermoSaveData data;
  DecodeThermoSaveData(&data, rtcDataBuf);

  if (data.magic == THERMO_STATE_DATA_MAGIC) {
    SetThermoSaveData(data);
  }

  ThermoButtonState buttons;
  DriverGetButtonStateNow(&buttons);
  SetThermoButtonState(buttons);

  if (buttons.FanState == FanModeT::On) {
    DriverRelayOn(Relay::Fan);
  }
}

void Machine::receive(const Event::Base& event) {
  auto newState = CurrentState.get_address<State::Base>()->handle_event(event);

  if (newState != get_state_id() && newState != State::Type::NO_CHANGE) {
    SwitchState(newState);
  }
}

void Machine::start(bool restart) {
  ::new (CurrentState.get_address<void*>()) Idle(*this);
}

State::Type Machine::get_state_id() const {
  return CurrentState.get_reference<State::Base>().StateId;
}

void Machine::SwitchState(State::Type new_state) {
  State::Base* address = CurrentState.get_address<State::Base>();

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
    case State::Type::Program:
      ::new (address) Program(*this);
      break;
    case State::Type::NO_CHANGE:
      break;
    case State::Type::COUNT:
      break;
  }
}
