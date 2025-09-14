#include "machine.hpp"
#include "state.hpp"

class Heating : public State::Base {
 public:
  Heating(Machine& machine);
  ~Heating();
  State::Type::TheType handle_event(const Event::Base& event) override;

 private:
  Machine& machine_;
};