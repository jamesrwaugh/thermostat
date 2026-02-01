#pragma once

#include <ThermoSaveData_bp.h>
#include <etl/alignment.h>
#include <etl/largest.h>

#include <driver_rs_wrapper.hpp>

#include "HomeAssistantSerial.hpp"
#include "coolable_parent.hpp"
#include "cooling.hpp"
#include "event.hpp"
#include "heating.hpp"
#include "idle.hpp"
#include "program_screen.hpp"
#include "state.hpp"

struct HaCommand;

class SafeThermoSaveData {
 public:
  SafeThermoSaveData();
  SafeThermoSaveData(const ThermoSaveData& other);
  ThermoSaveData Data;
  TemperatureUnitT TemperatureUnit() const;
  HeatModeT HeatMode() const;
  FanModeT FanMode() const;
  FanModeT BumpFanMode();
  HeatModeT BumpHeatingMode();
};

struct ProgramAutoTimeData {
  uint8_t Selection_{0};
};

class Machine {
 public:
  void SetThermoSaveData(const ThermoSaveData& raw);
  void SetThermoButtonState(const ThermoButtonState& raw);
  [[nodiscard]] ThermoButtonState& ButtonState();
  [[nodiscard]] const SafeThermoSaveData& SafeSaveState() const;
  [[nodiscard]] SafeThermoSaveData& SafeSaveState();
  void ReadAndApplySettings();
  void DisplayTemperature();
  void DisplaySetPointAndTemp();
  void ResetAutoTimeData();
  ProgramAutoTimeData& AutoTimeData();
  void ReadTemperatureAndReportIfChanged();
  uint8_t LastReadTemerature() const;

  void start();
  void receive(const Event::Base& event);
  void receive(const HaCommand& command);
  [[nodiscard]] State::Type get_state_id() const;

  // Home Assist integration
  void WriteHaTempStateTopicResponse(uint8_t temp) const;
  void WriteHaActionStateTopicResponse(HaActionKey key) const;
  void WriteHaModeStateTopicResponse() const;
  void WriteHaFanModeTopicResponse() const;

 private:
  void SwitchState(State::Type new_state, Event::Type lastEvent);
  void SaveProgrammingSettings();
  void WriteHaSerialResponse(HaOutTopicKey topic, uint8_t byte_one,
                             uint8_t byte_two) const;

  SafeThermoSaveData SaveData;
  ThermoButtonState ButtonData;
  ProgramAutoTimeData AtData;

  uint8_t LastReadTemp{0};
  uint8_t LastReadHumidity{0};
  uint8_t LastCommTemp{0};
  uint8_t LastCommHumidity{0};

  static constexpr size_t StatesMaxSize =
      etl::largest<Heating, Cooling, TempScreen, DateScreen, TimeScreen,
                   CoolableParent, Idle>::size;

  static constexpr size_t StatesAlignment =
      etl::largest<Heating, Cooling, TempScreen, DateScreen, TimeScreen,
                   CoolableParent, Idle>::alignment;

  etl::aligned_storage<StatesMaxSize, StatesAlignment>::type CurrentState;
};
