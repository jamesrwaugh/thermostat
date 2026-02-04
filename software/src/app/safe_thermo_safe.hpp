#pragma once

#include <ThermoSaveData_bp.h>

#include <driver_rs_wrapper.hpp>

class SafeThermoSaveData {
 public:
  SafeThermoSaveData();
  SafeThermoSaveData(const ThermoSaveData& other);

  TemperatureUnitT TemperatureUnit() const;
  HeatModeT HeatMode() const;
  FanModeT FanMode() const;
  uint8_t SetPoint() const;

  TemperatureUnitT& TemperatureUnit();
  HeatModeT& HeatMode();
  FanModeT& FanMode();
  uint8_t& SetPoint();

  const ThermoSaveData& Raw() const;
  ThermoSaveData& MutableRaw();

  FanModeT BumpFanMode();
  HeatModeT BumpHeatingMode();

  uint8_t Checksum() const {
    return 0;
  }

 private:
  ThermoSaveData Data;
};
