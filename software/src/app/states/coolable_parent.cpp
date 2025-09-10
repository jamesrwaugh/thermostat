#include "coolable_parent.hpp"

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
#include "state.hpp"

etl::fsm_state_id_t CoolableParent::on_enter_state() {
  auto& ctx = get_fsm_context();

  ctx.ResetStateChangeData();

  ThermoButtonState buttons;
  DriverGetButtonStateNow(&buttons);
  ctx.SetThermoButtonState(buttons);

  DriverDisplaySetPoint(ctx.SaveState().set_point);

  ctx.ReadTemperature();

  return No_State_Change;
}

void CoolableParent::on_exit_state() {
  get_fsm_context().ResetStateChangeData();
}

etl::fsm_state_id_t CoolableParent::on_event(const Event::UpButtonPressed&) {
  DriverWriteSerialPortLine("Up2");
  return get_fsm_context().ChangeSetPoint(1);
}

etl::fsm_state_id_t CoolableParent::on_event(const Event::DownButtonPressed&) {
  return get_fsm_context().ChangeSetPoint(-1);
}

etl::fsm_state_id_t CoolableParent::on_event(const Event::SecondPassed&) {
  auto& ctx = get_fsm_context();

  ctx.ReadTemperature();
  ctx.TickChangeCounter();

  return ctx.DetermineNextState();
}

etl::fsm_state_id_t CoolableParent::on_event(
    const Event::FanModeChanged& event) {
  auto& ctx = get_fsm_context();

  ctx.ButtonState().FanState = event.Mode;

  if (event.Mode == FanModeT::On) {
    DriverRelayOn(Relay::Fan);
  } else if (event.Mode == FanModeT::Auto && !ctx.IsHeatingOrCoolingNow()) {
    DriverRelayOff(Relay::Fan);
  }

  return No_State_Change;
}

etl::fsm_state_id_t CoolableParent::on_event(
    const Event::HeatModeChanged& event) {
  get_fsm_context().ButtonState().HeatingState = event.Mode;
  return get_fsm_context().DetermineNextState();
}

etl::fsm_state_id_t CoolableParent::on_event(
    const Event::ReverseValveModeChanged& event) {
  get_fsm_context().ButtonState().ReverseValveState = event.Mode;
  return State::Type::Idle;
}

etl::fsm_state_id_t CoolableParent::on_event_unknown(const etl::imessage&) {
  return No_State_Change;
}