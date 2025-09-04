#include <etl/hfsm.h>

#include "event.hpp"
#include "machine.hpp"
#include "protos/ThermoSaveData_bp.h"
#include "state.hpp"

class Program : public etl::fsm_state<Machine, Program, State::Type::Program> {
 public:
  etl::fsm_state_id_t on_enter_state() override;
  void on_exit_state() override;
  etl::fsm_state_id_t on_event(const Event::SecondPassed&);
  etl::fsm_state_id_t on_event_unknown(const etl::imessage&);

 private:
  ThermoSaveData DirtyData_;
};