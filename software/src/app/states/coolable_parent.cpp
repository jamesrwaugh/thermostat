#include "coolable_parent.hpp"

#include <driver_rs_wrapper.hpp>

#include "event.hpp"

etl::fsm_state_id_t CoolableParent::on_enter_state() {
  get_fsm_context().ResetStateChangeData();
  return No_State_Change;
}

void CoolableParent::on_exit_state() {
  get_fsm_context().ResetStateChangeData();
}

etl::fsm_state_id_t CoolableParent::on_event(const Event::UpButtonPressed&) {
  return get_fsm_context().ChangeSetPoint(1);
}

etl::fsm_state_id_t CoolableParent::on_event(const Event::DownButtonPressed&) {
  return get_fsm_context().ChangeSetPoint(-1);
}

etl::fsm_state_id_t CoolableParent::on_event(const Event::SecondPassed&) {
  get_fsm_context().TickChangeCounter();
  return get_fsm_context().DetermineNextState();
}

etl::fsm_state_id_t CoolableParent::on_event(
    const Event::FanModeChanged& event) {
  auto& ctx = get_fsm_context();

  ctx.ThermoStateData().FanMode() = event.Mode;

  if (event.Mode == Event::FanModeT::On) {
    DriverRelayOn(Relay::Fan);
  } else if (event.Mode == Event::FanModeT::Auto &&
             !ctx.IsHeatingOrCoolingNow()) {
    DriverRelayOff(Relay::Fan);
  }

  return No_State_Change;
}

etl::fsm_state_id_t CoolableParent::on_event(
    const Event::HeatModeChanged& event) {
  get_fsm_context().ThermoStateData().HeatingMode() = event.Mode;
  return get_fsm_context().DetermineNextState();
}

etl::fsm_state_id_t CoolableParent::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}