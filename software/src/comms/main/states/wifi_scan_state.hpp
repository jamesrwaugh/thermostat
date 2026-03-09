#pragma once

#include "state.hpp"

class Machine;

class WifiScanState : public State::Base {
 public:
  WifiScanState(Machine& m);
};