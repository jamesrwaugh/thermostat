#pragma once

#include "state.hpp"
#include "states/coolable_parent.hpp"

class Machine;

class Idle : public CoolableParent {
 public:
  Idle(Machine& machine);
  virtual ~Idle();
  State::Type handle_event(const Event::Base& event) override;
};