#include "safe_thermo_safe.hpp"

#include <string.h>

#include <casts.hpp>

#include "temperature.hpp"

// ===================================================================== //

SafeThermoSaveData::SafeThermoSaveData() {
  // It's real
  Data.magic = SAVEDATA_MAGIC;

  // Good defaults
  Data.set_point_mibicelcius = Temperature::FromCelcius(23).GetMibiCelcius();
  Data.temp_display_unit = TEMP_UNIT_FREEDOM;
  Data.fan_mode = FANMODE_AUTO;
  Data.heat_mode = HEATMODE_NONE;

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

Temperature SafeThermoSaveData::SetPoint() const {
  return Temperature::FromMibiCelcius(Data.set_point_mibicelcius);
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

void SafeThermoSaveData::SetSetPoint(Temperature t) {
  Data.set_point_mibicelcius = t.GetMibiCelcius();
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
