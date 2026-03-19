#pragma once

#include <ThermoSaveData_bp.h>
#include <stdint.h>

enum class FanModeT : uint8_t {
  On = FANMODE_ON,
  Auto = FANMODE_AUTO,
};

enum class HeatModeT : uint8_t {
  Heating = HEATMODE_HEAT,
  Cooling = HEATMODE_COOL,
  None = HEATMODE_NONE,
};

enum class ReverseValveModeT : uint8_t {
  OnForHeating = REVERSEVALVE_ON_HEAT,
  OnForCooling = REVERSEVALVE_ON_COOL,
};

enum class TemperatureUnitT : uint8_t {
  Freedom = TEMP_UNIT_FREEDOM,
  Celsius = TEMP_UNIT_CELSIUS,
};

enum class Relay : uint8_t {
  Fan = 0,
  Compressor,
  Heat,
  ReversingValve,
};

enum class Button : uint8_t {
  Up = 0,
  Down,
  Select,
  Fan,
  Heat,
};
