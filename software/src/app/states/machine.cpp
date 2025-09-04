#include "machine.hpp"

#include <etl/hfsm.h>

#include <driver_rs_wrapper.hpp>

#include "state.hpp"

Machine::Machine() : etl::hfsm(0) {}

void Machine::SetThermoStateData(const struct ThermoStateData& raw) {
  ::new (&Data) Event::SmartThermoStateData(raw);
}

[[nodiscard]] Event::SmartThermoStateData& Machine::ThermoStateData() {
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
  auto& setPoint = ThermoStateData().SetPoint();

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
  const auto& data = ThermoStateData();

  if (!HasChangeTimeoutPassed()) {
    return etl::ifsm_state::No_State_Change;
  }

  const uint8_t temp = DriverReadTemp();
  const auto heatMode = data.HeatingMode();
  const auto setPoint = data.SetPoint();

  if (heatMode == Event::HeatModeT::None) {
    return State::Type::Idle;
  }

  if (temp < setPoint && heatMode == Event::HeatModeT::Heating) {
    return State::Type::Heating;
  } else if (temp > setPoint && heatMode == Event::HeatModeT::Cooling) {
    return State::Type::Cooling;
  }

  return etl::ifsm_state::No_State_Change;
}

void Machine::ActivateCoolingRelays(Relay onRelay, Relay offRelay,
                                    ReverseValveTypeE onIfType) {
  ThermostatData data;
  DriverGetThermostatType(&data);

  if (data.ReverseValveType == onIfType) {
    DriverRelayOn(Relay::ReversingValve);
  } else {
    DriverRelayOff(Relay::ReversingValve);
  }

  DriverRelayOn(onRelay);
  DriverRelayOff(offRelay);
}

void Machine::EnterHeatingOrCooling() {
  ChData.IsHeatingOrCoolingNow = true;

  if (ThermoStateData().FanMode() == Event::FanModeT::Auto) {
    DriverRelayOn(Relay::Fan);
  }
}

void Machine::ExitHeatingOrCooling() {
  ResetStateChangeData();
  ChData.IsHeatingOrCoolingNow = false;

  if (ThermoStateData().FanMode() == Event::FanModeT::Auto) {
    DriverRelayOff(Relay::Fan);
  }
}

bool Machine::IsHeatingOrCoolingNow() const {
  return ChData.IsHeatingOrCoolingNow;
}