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
#include "mqtt/mqtt_state.hpp"
#include "program_screen.hpp"
#include "render/renderer.hpp"
#include "render/scroll_manager.hpp"
#include "safe_thermo_safe.hpp"
#include "state.hpp"
#include "temperature.hpp"

struct ProgramData {
  ds1307_time_s original_time;
  ds1307_time_s changed_time;
  TemperatureUnitT original_temp_unit;

  void Init(SafeThermoSaveData& data);
  bool WasTimeChanged() const;

  static_assert(sizeof(original_time) == sizeof(changed_time),
                "Time sizes are messed up for memcmp");
};

class RenderContext {
 public:
  RenderContext()
      : renderer_(DriverGetScreenHandle(), images_),
        temperature_manager_(images_.temperature_hundreds_or_minus_,
                             images_.temperature_tens_,
                             images_.temperature_ones_),
        humidity_manager_(dummy_humidity_hundreds_,
                          images_.humidity_tens_,
                          images_.humidity_ones_) {}

  Renderer renderer_;
  ScrollManager temperature_manager_;
  ScrollManager humidity_manager_;

 private:
  Image2x dummy_humidity_hundreds_;
  DigitImages images_;
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
  ProgramData& AutoTimeData();
  void ReadTemperature();
  void ReadTemperatureAndReportIfChanged();
  const Temperature& CurrentTemperature() const;
  const Humidity& CurrentHumidity() const;

  // Home Assist integration
  void WriteHaTempStateTopicResponse() const;
  void WriteHaHumidityStateTopicResponse() const;
  void WriteHaActionStateTopicResponse(HaActionKey key) const;
  void WriteHaModeStateTopicResponse() const;
  void WriteHaFanModeTopicResponse() const;

  RenderContext& GetRenderContext() {
    return rctx_;
  }

 private:
  void SwitchState(State::Type new_state, Event::Type lastEvent);
  void InitialRender();
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

  RenderContext rctx_;

  static constexpr size_t StatesMaxSize = etl::
      largest<Idle, Heating, Cooling, TempScreen, DateScreen, TimeScreen>::size;

  static constexpr size_t StatesAlignment =
      etl::largest<Idle, Heating, Cooling, TempScreen, DateScreen, TimeScreen>::
          alignment;

  etl::aligned_storage<StatesMaxSize, StatesAlignment>::type CurrentState;
};
