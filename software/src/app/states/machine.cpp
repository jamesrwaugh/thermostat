#include "machine.hpp"

#include <etl/hfsm.h>

#include <driver_rs_wrapper.hpp>

#include "protos/ThermoSaveData_bp.h"
#include "state.hpp"

Machine::Machine() : etl::hfsm(0) {}

void Machine::SetThermoSaveData(const ThermoSaveData& raw) {
  SaveData = raw;
}

void Machine::SetThermoButtonState(const ThermoButtonState& raw) {
  Data = raw;
}

[[nodiscard]] ThermoSaveData& Machine::SaveState() {
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

  return DetermineNextState();
}

[[nodiscard]] etl::fsm_state_id_t Machine::DetermineNextState() {
  const auto& buttons = ButtonState();

  if (!HasChangeTimeoutPassed()) {
    return etl::ifsm_state::No_State_Change;
  }

  const uint8_t temp = DriverReadTemp();
  const auto heatMode = buttons.HeatingState;
  const auto setPoint = SaveData.Data.set_point;

  if (heatMode == HeatModeT::None) {
    return State::Type::Idle;
  }

  if (temp < setPoint && heatMode == HeatModeT::Heating) {
    return State::Type::Heating;
  } else if (temp > setPoint && heatMode == HeatModeT::Cooling) {
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

void Machine::EnterHeatingOrCooling() {
  ChData.IsHeatingOrCoolingNow = true;

  if (ButtonState().FanState == FanModeT::Auto) {
    DriverRelayOn(Relay::Fan);
  }
}

void Machine::ExitHeatingOrCooling() {
  ResetStateChangeData();
  ChData.IsHeatingOrCoolingNow = false;

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
