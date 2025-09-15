#pragma once

#include "coolable_parent.hpp"
#include "state.hpp"

class Machine;

class Cooling : public CoolableParent {
 public:
  Cooling(Machine& machine);
  virtual ~Cooling();
  State::Type handle_event(const Event::Base& event) override;
};