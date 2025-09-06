#include "machine.hpp"

#include <etl/hfsm.h>

#include <driver_rs_wrapper.hpp>

#include "protos/ThermoCommEvent_bp.h"
#include "protos/ThermoSaveData_bp.h"
#include "state.hpp"

Machine::Machine() : etl::hfsm(0), LastReadTemp(0), LastCommTemp(0) {}

void Machine::SetThermoSaveData(const ThermoSaveData& raw) {
  SaveData = raw;
}

void Machine::SetThermoButtonState(const ThermoButtonState& raw) {
  Data = raw;
}

[[nodiscard]] ThermoSaveData& Machine::SaveState() {
  return SaveData.Data;
}

[[nodiscard]] const ThermoSaveData& Machine::SaveState() const {
  return SaveData.Data;
}

[[nodiscard]] ThermoButtonState& Machine::ButtonState() {
  return Data;
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
    LastCommTemp = LastReadTemp;
  }
}

[[nodiscard]] bool Machine::HasChangeTimeoutPassed() const {
  return ChData.StateChangeTimeoutSec >= ChData.MaxStateChangeTimeoutSec;
}

[[nodiscard]] etl::fsm_state_id_t Machine::ChangeSetPoint(int8_t change) {
  auto& setPoint = SaveData.Data.set_point;

  if (setPoint == 1 && change < 0) {
    return etl::ifsm_state::No_State_Change;
  }

  if (setPoint == 100 && change > 0) {
    return etl::ifsm_state::No_State_Change;
  }

  setPoint += change;

  DriverDisplaySetPoint(setPoint);

  auto v = SetPointChangedEvent{.new_set_point_f = setPoint};
  Comms()(v);

  return DetermineNextState();
}

[[nodiscard]] etl::fsm_state_id_t Machine::DetermineNextState() {
  const auto& buttons = ButtonState();

  if (!HasChangeTimeoutPassed()) {
    return etl::ifsm_state::No_State_Change;
  }

  const auto heatMode = buttons.HeatingState;
  const auto setPoint = SaveData.Data.set_point;

  if (heatMode == HeatModeT::None) {
    return State::Type::Idle;
  }

  if (LastReadTemp < setPoint && heatMode == HeatModeT::Heating) {
    return State::Type::Heating;
  } else if (LastReadTemp > setPoint && heatMode == HeatModeT::Cooling) {
    return State::Type::Cooling;
  }

  return etl::ifsm_state::No_State_Change;
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
  ChData.IsHeatingOrCoolingNow = true;

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
  ChData.IsHeatingOrCoolingNow = false;

  DriverRelayOff(Relay::Heat);
  DriverRelayOff(Relay::Compressor);
  DriverRelayOff(Relay::ReversingValve);

  if (ButtonState().FanState == FanModeT::Auto) {
    DriverRelayOff(Relay::Fan);
  }
}

bool Machine::IsHeatingOrCoolingNow() const {
  return ChData.IsHeatingOrCoolingNow;
}

SafeThermoSaveData::SafeThermoSaveData() {
  Data.magic = THERMO_STATE_DATA_MAGIC;
  Data.set_point = 70;
}

SafeThermoSaveData::SafeThermoSaveData(const ThermoSaveData& other) {
  Data = other;
}

void SerialPrintVisitor::operator()(TempChangedEvent& e) {
  uint8_t b[BYTES_LENGTH_TEMP_CHANGED_EVENT + 1];
  b[0] = 0;
  EncodeTempChangedEvent(&e, b + 1);
  DriverWriteSerialPort(b, sizeof(b));
}

void SerialPrintVisitor::operator()(SetPointChangedEvent& e) {
  uint8_t b[BYTES_LENGTH_SET_POINT_CHANGED_EVENT + 1];
  b[0] = 1;
  EncodeSetPointChangedEvent(&e, b + 1);
  DriverWriteSerialPort(b, sizeof(b));
}

void SerialPrintVisitor::operator()(HeatingModeChangedEvent& e) {
  uint8_t b[BYTES_LENGTH_HEATING_MODE_CHANGED_EVENT + 1];
  b[0] = 2;
  EncodeHeatingModeChangedEvent(&e, b + 1);
  DriverWriteSerialPort(b, sizeof(b));
}

void SerialPrintVisitor::operator()(ThermoSaveData& e) {
  uint8_t b[BYTES_LENGTH_THERMO_SAVE_DATA + 1];
  b[0] = 3;
  EncodeThermoSaveData(&e, b + 1);
  DriverWriteSerialPort(b, sizeof(b));
}

SerialPrintVisitor& Machine::Comms() {
  return V;
}
