#include "running_parent.hpp"

#include <driver_rs_wrapper.hpp>

#include "states/machine.hpp"

etl::fsm_state_id_t RunningParent::on_enter_state() {
  return No_State_Change;
}

void RunningParent::on_exit_state() {
  get_fsm_context().ResetStateChangeData();
}

etl::fsm_state_id_t RunningParent::on_event(const Event::UpButtonPressed&) {
  return ChangeSetPoint(1);
}

etl::fsm_state_id_t RunningParent::on_event(const Event::DownButtonPressed&) {
  return ChangeSetPoint(-1);
}

etl::fsm_state_id_t RunningParent::on_event(const Event::SecondPassed&) {
  return ChangeStateIfNeeded();
}

etl::fsm_state_id_t RunningParent::on_event(
    const Event::FanModeChanged& event) {
  get_fsm_context().ThermoStateData().FanMode() = event.Mode;
  return ChangeStateIfNeeded();
}

etl::fsm_state_id_t RunningParent::on_event(
    const Event::HeatModeChanged& event) {
  get_fsm_context().ThermoStateData().HeatingMode() = event.Mode;
  return ChangeStateIfNeeded();
}

etl::fsm_state_id_t RunningParent::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}

etl::fsm_state_id_t RunningParent::ChangeSetPoint(int8_t change) {
  auto& setPoint = get_fsm_context().ThermoStateData().SetPoint();

  if (setPoint == 1 && change < 0) {
    return No_State_Change;
  }

  if (setPoint == 100 && change > 0) {
    return No_State_Change;
  }

  setPoint += change;

  DriverDisplaySetPoint(setPoint);

  return ChangeStateIfNeeded();
}

etl::fsm_state_id_t RunningParent::ChangeStateIfNeeded() {
  auto& data = get_fsm_context().ThermoStateData();

  if (!get_fsm_context().HasChangeTimeoutPassed()) {
    return No_State_Change;
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

  return No_State_Change;
}