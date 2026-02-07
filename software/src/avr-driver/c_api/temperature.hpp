#pragma once

#include <stdint.h>

#include "../data_types.hpp"

class Temperature {
 public:
  [[nodiscard]] static Temperature FromCelcius(uint16_t celcius) {
    Temperature t;
    t.SetFromCelcius(celcius);
    return t;
  }

  [[nodiscard]] static Temperature FromMibiCelcius(uint16_t mibicelcius) {
    Temperature t;
    t.SetFromMibiCelcius(mibicelcius);
    return t;
  }

  void SetFromMibiCelcius(uint16_t mibi_celcius) {
    mibi_celcius_ = mibi_celcius;
  }

  void SetFromCelcius(uint16_t celcius) {
    mibi_celcius_ = celcius * MibiFactor;
  }

  void SetFromSht4xSensor(uint16_t device_ticks) {
    mibi_celcius_ = ((22411 * (int32_t)device_ticks) >> 13) - 46080;
  }

  int8_t GetUnitWhole(TemperatureUnitT unit) const {
    return unit == TemperatureUnitT::Celsius ? GetCelciusWhole()
                                             : GetFahrenheitWhole();
  }

  int16_t GetMibiCelcius() const {
    return mibi_celcius_;
  }

  void ChangeBy1Unit(TemperatureUnitT unit, bool increment) {
    unit == TemperatureUnitT::Celsius ? ChangeBy1C(increment)
                                      : ChangeBy1F(increment);
  }

  int8_t operator<=>(const Temperature& other) const {
    if (mibi_celcius_ < other.mibi_celcius_) return -1;
    if (mibi_celcius_ > other.mibi_celcius_) return 1;
    return 0;
  }

  bool operator==(const Temperature& other) const {
    return mibi_celcius_ == other.mibi_celcius_;
  }

 private:
  static constexpr uint16_t MibiFactor = 1024;
  static constexpr uint16_t MibiCToFFactor = 568;

  void ChangeBy1C(bool increment) {
    mibi_celcius_ += (increment ? MibiFactor : -MibiFactor);
  }

  void ChangeBy1F(bool increment) {
    mibi_celcius_ += (increment ? MibiCToFFactor : -MibiCToFFactor);
  }

  int8_t GetCelciusWhole() const {
    return mibi_celcius_ / MibiFactor;
  }

  int8_t GetFahrenheitWhole() const {
    int32_t mibi_fahrenheight = mibi_celcius_;
    mibi_fahrenheight <<= 3;
    mibi_fahrenheight += mibi_celcius_;
    mibi_fahrenheight /= 5;
    mibi_fahrenheight += (32u * MibiFactor);
    return mibi_fahrenheight / MibiFactor;
  }

  // Degrees Celsius * 1024.
  // This allows us to store fractional temperatures as well
  // as only require bitshifts to encode and decode whole values,
  // to save on flash size, instead of divide by 1000 for example.
  int16_t mibi_celcius_{0};
};

static_assert(sizeof(Temperature) <= 2,
              "Temperature should be small enough to pass by value");
