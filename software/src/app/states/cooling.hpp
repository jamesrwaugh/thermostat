#pragma once

#include "coolable_parent.hpp"

class Machine;

class Cooling : public CoolableParent {
 public:
  Cooling(Machine& machine);
  State::Type handle_event(const Event::Base& event) override;
  virtual ~Cooling();

 private:
  bool image_state_{false};
};
