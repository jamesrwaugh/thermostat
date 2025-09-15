#pragma once

#include "coolable_parent.hpp"
#include "state.hpp"

class Machine;

class Heating : public CoolableParent {
 public:
  Heating(Machine& machine);
  virtual ~Heating();
  State::Type handle_event(const Event::Base& event) override;
};