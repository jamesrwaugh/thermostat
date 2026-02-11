#pragma once

#include <driver_ds1307.h>
#include <etl/alignment.h>
#include <etl/largest.h>

#include <HomeAssistantSerial.hpp>
#include <driver_rs_wrapper.hpp>

#include "cooling.hpp"
#include "event.hpp"
#include "heating.hpp"
#include "idle.hpp"
#include "program_screen.hpp"
#include "safe_thermo_safe.hpp"
#include "state.hpp"
#include "temperature.hpp"

struct ProgramData {
  uint8_t Selection_{0};
  ds1307_time_s StartTime_;
  ds1307_time_s ChangedTime_;

  bool WasTimeChanged() const {
    return memcmp(&StartTime_, &ChangedTime_, sizeof(StartTime_)) != 0;
  }

  static_assert(sizeof(StartTime_) == sizeof(ChangedTime_),
                "Time sizes are messed up for memcmp");
};

struct MqttState {
  static constexpr uint8_t gMqttTimeoutMaxSeconds = 60;

  void IncreaseTimeout() {
    if (MqttDisconnectedSeconds < gMqttTimeoutMaxSeconds) {
      MqttDisconnectedSeconds += 1;
    }
  }

  void ResetTimeout() {
    MqttDisconnectedSeconds = 0;
  }

  bool IsMqttConnected() const {
    return MqttDisconnectedSeconds < gMqttTimeoutMaxSeconds;
  }

 private:
  enum class SmartMode : uint8_t {
    Controller,
    HomeAssistant,
  };

  SmartMode Mode{SmartMode::Controller};
  uint8_t MqttDisconnectedSeconds{0};
};

struct TemperatureChangeInfo {
  bool TemperatureChanged;
  bool HumidityChanged;
  int8_t TemperatureChangeDirection;
  int8_t HumidChangedDirection;
};

class Machine {
 public:
  // State machine control
  void start();
  void receive(const Event::Base& event);
  void receive(const HaCommand& command);
  [[nodiscard]] State::Type get_state_id() const;

  // Themostat operations
  [[nodiscard]] ThermoButtonState& ButtonState();
  [[nodiscard]] const SafeThermoSaveData& SaveState() const;
  [[nodiscard]] SafeThermoSaveData& SaveState();
  void ReadAndApplySettings();
  void DisplaySetPoint();
  void ResetAutoTimeData();
  ProgramData& AutoTimeData();
  void ReadTemperature();
  void ReadTemperatureAndReportIfChanged(TemperatureChangeInfo& info);
  const Temperature& CurrentTemperature() const;
  const Humidity& CurrentHumidity() const;

  // Home Assist integration
  void WriteHaTempStateTopicResponse() const;
  void WriteHaHumidityStateTopicResponse() const;
  void WriteHaActionStateTopicResponse(HaActionKey key) const;
  void WriteHaModeStateTopicResponse() const;
  void WriteHaFanModeTopicResponse() const;

 private:
  void SwitchState(State::Type new_state, Event::Type lastEvent);
  void SetupProgramming();
  void SaveProgrammingSettings();
  void ApplySaveState();
  void SetThermoButtonState(const ThermoButtonState& raw);
  void WriteHaSerialResponse(HaOutTopicKey topic,
                             uint8_t byte_one,
                             uint8_t byte_two) const;

  SafeThermoSaveData SaveData;
  ThermoButtonState ButtonData;
  ProgramData ProgData;
  MqttState MqttData;

  Temperature CurrentTemp;
  Temperature PreviousTemp;
  Humidity CurrentHumid;
  Humidity PreviousHumidity;

  static constexpr size_t StatesMaxSize = etl::
    largest<Idle, Heating, Cooling, TempScreen, DateScreen, TimeScreen>::size;

  static constexpr size_t StatesAlignment =
    etl::largest<Idle, Heating, Cooling, TempScreen, DateScreen, TimeScreen>::
      alignment;

  etl::aligned_storage<StatesMaxSize, StatesAlignment>::type CurrentState;
};
