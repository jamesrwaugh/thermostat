#pragma once

#include "states/coolable_parent.hpp"

class Machine;

class Idle : public CoolableParent {
 public:
  Idle(Machine& machine);
};
