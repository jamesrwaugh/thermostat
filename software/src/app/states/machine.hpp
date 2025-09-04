#pragma once

#include <etl/hfsm.h>

#include <driver_rs_wrapper.hpp>

#include "protos/ThermoSaveData_bp.h"

class SafeThermoSaveData {
 public:
  SafeThermoSaveData();
  SafeThermoSaveData(const ThermoSaveData& other);
  ThermoSaveData Data;
};

class Machine : public etl::hfsm {
 public:
  Machine();

  void SetThermoSaveData(const ThermoSaveData& raw);
  void SetThermoButtonState(const ThermoButtonState& raw);
  [[nodiscard]] ThermoButtonState& ButtonState();
  [[nodiscard]] ThermoSaveData& SaveState();
  void ResetStateChangeData();
  void TickChangeCounter();
  [[nodiscard]] bool HasChangeTimeoutPassed() const;
  [[nodiscard]] etl::fsm_state_id_t ChangeSetPoint(int8_t change);
  [[nodiscard]] etl::fsm_state_id_t DetermineNextState();
  void ActivateCoolingRelays(Relay onRelay, Relay offRelay,
                             ReverseValveModeT onIfType);
  void EnterHeatingOrCooling();
  void ExitHeatingOrCooling();
  bool IsHeatingOrCoolingNow() const;

 private:
  struct StateChangeData {
    static constexpr uint8_t MaxStateChangeTimeoutSec = 10;
    uint8_t StateChangeTimeoutSec{0};
    bool IsHeatingOrCoolingNow{false};
  };

  SafeThermoSaveData SaveData;
  ThermoButtonState Data;
  StateChangeData ChData;
};