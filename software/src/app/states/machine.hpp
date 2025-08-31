#pragma once

#include <etl/hfsm.h>

#include "event.hpp"
#include "protos/ThermoStateData_bp.h"

class Machine : public etl::hfsm {
 public:
  Machine() : etl::hfsm(0) {}

  void SetThermoStateData(const ThermoStateData& p) {
    new (&Data) Event::SmartThermoStateData(p);
  }

  Event::SmartThermoStateData Data;
};