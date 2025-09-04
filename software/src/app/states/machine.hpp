#pragma once

#include <etl/hfsm.h>

#include <driver_rs_wrapper.hpp>

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
  [[nodiscard]] etl::fsm_state_id_t ChangeSetPoint(int8_t change);
  [[nodiscard]] etl::fsm_state_id_t DetermineNextState();
  void ActivateCoolingRelays(Relay onRelay, Relay offRelay,
                             ReverseValveTypeE onIfType);
  void EnterHeatingOrCooling();
  void ExitHeatingOrCooling();
  bool IsHeatingOrCoolingNow() const;

 private:
  struct StateChangeData {
    static constexpr uint8_t MaxStateChangeTimeoutSec = 10;
    uint8_t StateChangeTimeoutSec{0};
    bool IsHeatingOrCoolingNow{false};
  };

  Event::SmartThermoStateData Data;
  StateChangeData ChData;
};