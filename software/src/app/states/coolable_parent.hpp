#include <etl/hfsm.h>

#include "event.hpp"
#include "machine.hpp"
#include "state.hpp"

class CoolableParent
    : public etl::fsm_state<Machine, CoolableParent, State::Type::Cooling> {
 public:
  etl::fsm_state_id_t on_enter_state() override;
  void on_exit_state() override;
  etl::fsm_state_id_t on_event(const Event::UpButtonPressed&);
  etl::fsm_state_id_t on_event(const Event::DownButtonPressed&);
  etl::fsm_state_id_t on_event(const Event::SecondPassed&);
  etl::fsm_state_id_t on_event(const Event::FanModeChanged& event);
  etl::fsm_state_id_t on_event(const Event::HeatModeChanged& event);
  etl::fsm_state_id_t on_event(const Event::ReverseValveModeChanged& event);
  etl::fsm_state_id_t on_event_unknown(const etl::imessage&);
};