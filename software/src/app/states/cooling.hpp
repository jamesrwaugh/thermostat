#include <etl/hfsm.h>

#include "machine.hpp"
#include "state.hpp"

class Cooling : public etl::fsm_state<Machine, Cooling, State::Type::Cooling> {
 public:
  etl::fsm_state_id_t on_enter_state() override;
  void on_exit_state() override;
  etl::fsm_state_id_t on_event(const Event::SecondPassed&);
  etl::fsm_state_id_t on_event_unknown(const etl::imessage&);
};