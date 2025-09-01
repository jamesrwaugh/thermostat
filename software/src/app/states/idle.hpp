#include <etl/hfsm.h>

#include "machine.hpp"
#include "state.hpp"

class Idle : public etl::fsm_state<Machine, Idle, State::Type::Idle> {
 public:
  etl::fsm_state_id_t on_enter_state() override;
  void on_exit_state() override;
  etl::fsm_state_id_t on_event(const Event::SecondPassed&);
  etl::fsm_state_id_t on_event_unknown(const etl::imessage&);
};