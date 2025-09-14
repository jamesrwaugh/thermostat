#include "machine.hpp"
#include "state.hpp"

class Idle : public State::Base {
 public:
  Idle(Machine& machine);
  ~Idle();
  State::Type::TheType handle_event(const Event::Base& event) override;

 private:
  Machine& machine_;
};