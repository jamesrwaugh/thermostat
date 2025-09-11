#pragma once

#include <stdint.h>

enum class FanModeT : uint8_t {
  On = 0,
  Auto = 1,
};

enum class HeatModeT : uint8_t {
  Heating = 0,
  Cooling = 1,
  None = 2,
};

enum class ReverseValveModeT : uint8_t {
  OnForHeating = 0,
  OnForCooling = 1,
};

enum class TemperatureUnitT : uint8_t {
  Freedom,
  Celsius,
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
  TempHeat,
  TempCold,
  TempNone,
  FanAuto,
  FanOn,
  ReverseValveOnHeat,
  ReverseValveOnCool,
};
