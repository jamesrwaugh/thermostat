#pragma once

#include <etl/hfsm.h>

#include <driver_rs_wrapper.hpp>

#include "protos/ThermoCommEvent_bp.h"
#include "protos/ThermoSaveData_bp.h"

class SafeThermoSaveData {
 public:
  SafeThermoSaveData();
  SafeThermoSaveData(const ThermoSaveData& other);
  ThermoSaveData Data;
};

struct SerialPrintVisitor {
  void operator()(TempChangedEvent& e);
  void operator()(SetPointChangedEvent& e);
  void operator()(HeatingModeChangedEvent& e);
  void operator()(ThermoSaveData& e);
};

class Machine : public etl::hfsm {
 public:
  Machine();

  void SetThermoSaveData(const ThermoSaveData& raw);
  void SetThermoButtonState(const ThermoButtonState& raw);
  [[nodiscard]] ThermoButtonState& ButtonState();
  [[nodiscard]] ThermoSaveData& SaveState();
  [[nodiscard]] const ThermoSaveData& SaveState() const;
  void ResetStateChangeData();
  void TickChangeCounter();
  [[nodiscard]] bool HasChangeTimeoutPassed() const;
  [[nodiscard]] etl::fsm_state_id_t ChangeSetPoint(int8_t change);
  [[nodiscard]] etl::fsm_state_id_t DetermineNextState();
  void ActivateCoolingRelays(Relay onRelay, Relay offRelay,
                             ReverseValveModeT onIfType);
  void EnterHeatingOrCooling(HeatModeT mode);
  void ExitHeatingOrCooling();
  void ReadTemperature();
  bool IsHeatingOrCoolingNow() const;
  SerialPrintVisitor& Comms();

 private:
  struct StateChangeData {
    static constexpr uint8_t MaxStateChangeTimeoutSec = 10;
    uint8_t StateChangeTimeoutSec{0};
  };

  SafeThermoSaveData SaveData;
  ThermoButtonState ButtonData;
  StateChangeData ChData;
  SerialPrintVisitor V;
  uint8_t LastReadTemp{0};
  uint8_t LastCommTemp{0};
};