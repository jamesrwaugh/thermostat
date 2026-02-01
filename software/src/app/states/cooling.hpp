#pragma once

#include "coolable_parent.hpp"

class Machine;

class Cooling : public CoolableParent {
 public:
  Cooling(Machine& machine);
  virtual ~Cooling();
};
