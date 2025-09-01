#pragma once

#include <etl/hfsm.h>

#include "event.hpp"
#include "protos/ThermoStateData_bp.h"

class Machine : public etl::hfsm {
 public:
  Machine();

  void SetThermoStateData(const ThermoStateData& raw);
  [[nodiscard]] Event::SmartThermoStateData& ThermoStateData();
  void ResetStateChangeData();
  void TickChangeCounter();
  [[nodiscard]] bool HasChangeTimeoutPassed() const;

 private:
  struct StateChangeData {
    static constexpr uint8_t MaxStateChangeTimeoutSec = 10;
    uint8_t StateChangeTimeoutSec{0};
  };

  Event::SmartThermoStateData Data;
  StateChangeData ChData;
};