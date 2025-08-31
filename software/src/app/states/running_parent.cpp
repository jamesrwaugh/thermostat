#include "running_parent.hpp"

#include <driver_rs_wrapper.hpp>

etl::fsm_state_id_t RunningParent::on_enter_state() {
  return No_State_Change;
}

etl::fsm_state_id_t RunningParent::on_event(const Event::UpButtonPressed&) {
  return SetSetPoint(-1);
}

etl::fsm_state_id_t RunningParent::on_event(const Event::DownButtonPressed&) {
  return SetSetPoint(1);
}

etl::fsm_state_id_t RunningParent::on_event(const Event::SecondPassed&) {
  return ChangeStateIfNeeded();
}

etl::fsm_state_id_t RunningParent::on_event(
    const Event::FanModeChanged& event) {
  auto& context = get_fsm_context();
  context.Data.FanMode() = event.Mode;
  return ChangeStateIfNeeded();
}

etl::fsm_state_id_t RunningParent::on_event(
    const Event::HeatModeChanged& event) {
  auto& context = get_fsm_context();
  context.Data.HeatingMode() = event.Mode;
  return ChangeStateIfNeeded();
}

etl::fsm_state_id_t RunningParent::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}

etl::fsm_state_id_t RunningParent::SetSetPoint(int8_t change) {
  auto& context = get_fsm_context();

  auto& setPoint = context.Data.SetPoint();

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
  auto& data = get_fsm_context().Data;

  uint8_t temp = DriverReadTemp();

  auto& heatMode = data.HeatingMode();
  auto setPoint = data.SetPoint();

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
