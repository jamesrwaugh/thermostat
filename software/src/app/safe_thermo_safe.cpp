#include "safe_thermo_safe.hpp"

#include <string.h>

#include "casts.hpp"

// ===================================================================== //

SafeThermoSaveData::SafeThermoSaveData() {
  // Good defaults
  Data.set_point = 77;
  Data.temp_display_unit = TEMP_UNIT_FREEDOM;
  Data.fan_mode = FANMODE_AUTO;
  Data.heat_mode = HEATMODE_NONE;

  // Default date of 2000-01-01, was Saturday
  Data.date.day = 1;
  Data.date.month = 1;
  Data.date.year = 0;
  Data.date.day_of_week = DAYOFWEEK_SATURDAY;

  // Default time of 12:00 AM
  Data.time.second = 0;
  Data.time.minute = 0;
  Data.time.hour = 0;
  Data.time.am_pm = TIME_AM;

  // Empty MQTT data
  Data.have_mqtt = false;
  memset(Data.mqtt.wifi_name, 0, sizeof(Data.mqtt.wifi_name));
  memset(Data.mqtt.wifi_password, 0, sizeof(Data.mqtt.wifi_password));
}

TemperatureUnitT SafeThermoSaveData::TemperatureUnit() const {
  return static_cast<TemperatureUnitT>(Data.temp_display_unit);
}

HeatModeT SafeThermoSaveData::HeatMode() const {
  return static_cast<HeatModeT>(Data.heat_mode);
}

FanModeT SafeThermoSaveData::FanMode() const {
  return static_cast<FanModeT>(Data.fan_mode);
}

uint8_t SafeThermoSaveData::SetPoint() const {
  return Data.set_point;
}

TemperatureUnitT& SafeThermoSaveData::TemperatureUnit() {
  return reinterpret_cast<TemperatureUnitT&>(Data.temp_display_unit);
}

HeatModeT& SafeThermoSaveData::HeatMode() {
  return reinterpret_cast<HeatModeT&>(Data.heat_mode);
}

FanModeT& SafeThermoSaveData::FanMode() {
  return reinterpret_cast<FanModeT&>(Data.fan_mode);
}

uint8_t& SafeThermoSaveData::SetPoint() {
  return Data.set_point;
}

const ThermoSaveData& SafeThermoSaveData::Raw() const {
  return Data;
}

ThermoSaveData& SafeThermoSaveData::MutableRaw() {
  return Data;
}

SafeThermoSaveData::SafeThermoSaveData(const ThermoSaveData& other) {
  Data = other;
}

HeatModeT SafeThermoSaveData::BumpHeatingMode() {
  switch (static_cast<HeatModeT>(Data.heat_mode)) {
    case HeatModeT::Heating:
      Data.heat_mode = u8(HeatModeT::Cooling);
      break;
    case HeatModeT::Cooling:
      Data.heat_mode = u8(HeatModeT::None);
      break;
    case HeatModeT::None:
      Data.heat_mode = u8(HeatModeT::Heating);
      break;
  }
  return static_cast<HeatModeT>(Data.heat_mode);
}

FanModeT SafeThermoSaveData::BumpFanMode() {
  switch (static_cast<FanModeT>(Data.fan_mode)) {
    case FanModeT::On:
      Data.fan_mode = u8(FanModeT::Auto);
      break;
    case FanModeT::Auto:
      Data.fan_mode = u8(FanModeT::On);
      break;
  }
  return static_cast<FanModeT>(Data.fan_mode);
}

// ===================================================================== //
