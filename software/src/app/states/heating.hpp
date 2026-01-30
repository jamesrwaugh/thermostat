#pragma once

#include "coolable_parent.hpp"

class Machine;

class Heating : public CoolableParent {
 public:
  Heating(Machine& machine);
  virtual ~Heating();
};
