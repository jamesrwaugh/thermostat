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
  context.FanMode = event.Mode;
  return ChangeStateIfNeeded();
}

etl::fsm_state_id_t RunningParent::on_event(
    const Event::HeatModeChanged& event) {
  auto& context = get_fsm_context();
  context.HeatMode = event.Mode;
  return ChangeStateIfNeeded();
}

etl::fsm_state_id_t RunningParent::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}

etl::fsm_state_id_t RunningParent::SetSetPoint(int8_t change) {
  auto& context = get_fsm_context();

  if (context.SetPoint == 1 && change < 0) {
    return No_State_Change;
  }

  if (context.SetPoint == 100 && change > 0) {
    return No_State_Change;
  }

  context.SetPoint += change;

  DriverDisplaySetPoint(context.SetPoint);

  return ChangeStateIfNeeded();
}

etl::fsm_state_id_t RunningParent::ChangeStateIfNeeded() {
  auto& context = get_fsm_context();

  uint8_t temp = DriverReadTemp();

  if (context.HeatMode == Event::HeatModeT::None) {
    return State::Type::Idle;
  }

  if (temp < context.SetPoint &&
      context.HeatMode == Event::HeatModeT::Heating) {
    return State::Type::Heating;
  } else if (temp > context.SetPoint &&
             context.HeatMode == Event::HeatModeT::Cooling) {
    return State::Type::Cooling;
  }

  return No_State_Change;
}
