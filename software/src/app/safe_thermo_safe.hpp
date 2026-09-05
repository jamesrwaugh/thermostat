#pragma once

#include <ThermoSaveData_bp.h>

#include <driver_rs_wrapper.hpp>

#include "temperature.hpp"

class SafeThermoSaveData {
 public:
  SafeThermoSaveData();
  SafeThermoSaveData(const ThermoSaveData& other);

  TemperatureUnitT TemperatureUnit() const;
  HeatModeT HeatMode() const;
  FanModeT FanMode() const;
  Temperature SetPoint() const;

  TemperatureUnitT& TemperatureUnit();
  HeatModeT& HeatMode();
  FanModeT& FanMode();

  void SetReverseValveMode(ReverseValveModeT mode);
  ReverseValveModeT ReverseValveMode() const;

  void SetSetPoint(const Temperature& t);

  const ThermoSaveData& Raw() const;
  ThermoSaveData& MutableRaw();

  FanModeT BumpFanMode();
  HeatModeT BumpHeatingMode();

 private:
  ThermoSaveData Data;
};
