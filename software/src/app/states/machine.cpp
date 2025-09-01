#include "machine.hpp"

Machine::Machine() : etl::hfsm(0) {}

void Machine::SetThermoStateData(const struct ThermoStateData& raw) {
  ::new (&Data) Event::SmartThermoStateData(raw);
}

[[nodiscard]] Event::SmartThermoStateData& Machine::ThermoStateData() {
  return Data;
}

void Machine::ResetStateChangeData() {
  ::new (&ChData) StateChangeData();
}

void Machine::TickChangeCounter() {
  if (ChData.StateChangeTimeoutSec < ChData.MaxStateChangeTimeoutSec) {
    ChData.StateChangeTimeoutSec += 1;
  }
}

[[nodiscard]] bool Machine::HasChangeTimeoutPassed() const {
  return ChData.StateChangeTimeoutSec >= ChData.MaxStateChangeTimeoutSec;
}
