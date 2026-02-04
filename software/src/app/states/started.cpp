#include "started.hpp"

Started::Started() : State::Base(State::Type::Started) {}

State::Type Started::handle_event(const Event::Base& event) {
  return State::Type::NO_CHANGE;
}
