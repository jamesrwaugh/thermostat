#pragma once

#include <etl/alignment.h>
#include <etl/largest.h>

#include <driver_rs_wrapper.hpp>

#include "coolable_parent.hpp"
#include "cooling.hpp"
#include "event.hpp"
#include "heating.hpp"
#include "idle.hpp"
#include "program.hpp"
#include "protos/ThermoCommEvent_bp.h"
#include "protos/ThermoSaveData_bp.h"
#include "state.hpp"

class SafeThermoSaveData {
 public:
  SafeThermoSaveData();
  SafeThermoSaveData(const ThermoSaveData& other);
  ThermoSaveData Data;
  TemperatureUnitT TemperatureUnit() const;
};

struct SerialPrintVisitor {
  void operator()(TempChangedEvent& e);
  void operator()(SetPointChangedEvent& e);
  void operator()(HeatingModeChangedEvent& e);
  void operator()(ThermoSaveData& e);
};

class Machine {
 public:
  Machine();

  void SetThermoSaveData(const ThermoSaveData& raw);
  void SetThermoButtonState(const ThermoButtonState& raw);
  [[nodiscard]] ThermoButtonState& ButtonState();
  [[nodiscard]] ThermoSaveData& SaveState();
  [[nodiscard]] const ThermoSaveData& SaveState() const;
  [[nodiscard]] const SafeThermoSaveData& SafeSaveState() const;
  void ResetStateChangeData();
  void TickChangeCounter();
  [[nodiscard]] bool HasChangeTimeoutPassed() const;
  [[nodiscard]] State::Type ChangeSetPoint(int8_t change);
  [[nodiscard]] State::Type DetermineNextState();
  void ActivateCoolingRelays(Relay onRelay, Relay offRelay,
                             ReverseValveModeT onIfType);
  void EnterHeatingOrCooling(HeatModeT mode);
  void ExitHeatingOrCooling();
  void ReadTemperature();
  bool IsHeatingOrCoolingNow() const;
  void ReadAndApplySettings();
  SerialPrintVisitor& Comms();

  // New state management methods
  void receive(const Event::Base& event);
  void start(bool restart = false);
  [[nodiscard]] State::Type get_state_id() const;

 private:
  void SwitchState(State::Type new_state);

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

  static constexpr size_t StatesMaxSize =
      etl::largest<Heating, Cooling, Program, CoolableParent, Idle>::size;

  static constexpr size_t StatesAlignment =
      etl::largest<Heating, Cooling, Program, CoolableParent, Idle>::alignment;

  etl::aligned_storage<StatesMaxSize, StatesAlignment>::type CurrentState;
};