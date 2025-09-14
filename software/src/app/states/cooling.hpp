#include "machine.hpp"
#include "state.hpp"

class Cooling : public State::Base {
 public:
  Cooling(Machine& machine);
  ~Cooling();
  State::Type::TheType handle_event(const Event::Base& event) override;

 private:
  Machine& machine_;
};