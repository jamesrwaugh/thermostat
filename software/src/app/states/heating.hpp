#pragma once

#include "coolable_parent.hpp"

class Machine;

class Heating : public CoolableParent {
 public:
  Heating(Machine& machine);
  virtual ~Heating();
  State::Type handle_event(const Event::Base& event) override;

 private:
  bool image_state_{false};
};
