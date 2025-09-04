#pragma once

#include <etl/message.h>

#include "protos/ThermoStateData_bp.h"

namespace Event {

struct Type {
  enum TheType : etl::message_id_t {
    UpButtonPressed = 0,
    DownButtonPressed,
    SelectButtonPressed,
    SecondPassed,
    FanModeChanged,
    HeatModeChanged,
  };
};

enum class FanModeT : uint8_t {
  On = FAN_ON,
  Auto = FAN_AUTO,
};

enum class HeatModeT : uint8_t {
  Heating = HEATING_HEATING,
  Cooling = HEATING_COOLING,
  None = HEATING_NONE,
};

class SmartThermoStateData {
  static_assert(etl::is_same<etl::underlying_type_t<FanModeT>, FanState>::value,
                "Types not same, needed for enum class cast");

  static_assert(
      etl::is_same<etl::underlying_type_t<HeatModeT>, HeatingState>::value,
      "Types not same, needed for enum class cast");

 public:
  SmartThermoStateData() {
    Data_.magic = THERMO_STATE_DATA_MAGIC;
    Data_.set_point = 70;
    Data_.fan_state = static_cast<FanState>(Event::FanModeT::Auto);
    Data_.heating_state = static_cast<HeatingState>(Event::HeatModeT::Heating);
  }

  SmartThermoStateData(const ThermoStateData& data) {
    Data_ = data;
    Data_.magic = THERMO_STATE_DATA_MAGIC;
  }

  Event::FanModeT& FanMode() {
    return (Event::FanModeT&)(Data_.fan_state);
  }

  const Event::FanModeT FanMode() const {
    return static_cast<Event::FanModeT>(Data_.fan_state);
  }

  Event::HeatModeT& HeatingMode() {
    return (Event::HeatModeT&)(Data_.heating_state);
  }

  const Event::HeatModeT HeatingMode() const {
    return static_cast<Event::HeatModeT>(Data_.heating_state);
  }

  uint8_t& SetPoint() {
    return Data_.set_point;
  }

  uint8_t SetPoint() const {
    return Data_.set_point;
  }

 private:
  ThermoStateData Data_;
};

struct UpButtonPressed : etl::message<Event::Type::UpButtonPressed> {};
struct DownButtonPressed : etl::message<Event::Type::DownButtonPressed> {};
struct SelectButtonPressed : etl::message<Event::Type::SelectButtonPressed> {};
struct SecondPassed : etl::message<Event::Type::SecondPassed> {};

struct FanModeChanged : etl::message<Event::Type::FanModeChanged> {
  FanModeChanged(FanModeT mode) : Mode(mode) {}
  const FanModeT Mode;
};

struct HeatModeChanged : etl::message<Event::Type::HeatModeChanged> {
  HeatModeChanged(HeatModeT mode) : Mode(mode) {}
  const HeatModeT Mode;
};

}  // namespace Event
