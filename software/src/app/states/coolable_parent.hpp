#include "event.hpp"
#include "machine.hpp"
#include "state.hpp"

class CoolableParent : public State::Base {
 public:
  CoolableParent(Machine& machine);
  ~CoolableParent();
  State::Type::TheType handle_event(const Event::Base& event) override;

 private:
  Machine& machine_;
};