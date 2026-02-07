#pragma once

#include <stdint.h>

#include "../data_types.hpp"

class Temperature {
 public:
  static Temperature FromCelcius(uint16_t celcius) {
    Temperature t;
    t.SetFromCelcius(celcius);
    return t;
  }

  void SetFromMibiCelcius(uint16_t mibi_celcius) {
    mibi_celcius_ = mibi_celcius;
  }

  void SetFromCelcius(uint16_t celcius) {
    mibi_celcius_ = celcius << 10;
  }

  void SetFromSht4xSensor(uint16_t device_ticks) {
    mibi_celcius_ = ((22411 * (int32_t)device_ticks) >> 13) - 46080;
  }

  int8_t GetUnitWhole(TemperatureUnitT unit) const {
    return unit == TemperatureUnitT::Celsius ? GetCelciusWhole()
                                             : GetFahrenheitWhole();
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
  void ChangeBy1C(bool increment) {
    mibi_celcius_ += (increment ? 1024 : -1024);
  }

  void ChangeBy1F(bool increment) {
    mibi_celcius_ += (increment ? 569 : -569);
  }

  int8_t GetCelciusWhole() const {
    return mibi_celcius_ >> 10;
  }

  int8_t GetFahrenheitWhole() const {
    int32_t mibi_fahrenheight = mibi_celcius_;
    mibi_fahrenheight <<= 3;
    mibi_fahrenheight += mibi_fahrenheight;
    mibi_fahrenheight /= 5;
    mibi_fahrenheight += (32u * 1024u);
    return mibi_fahrenheight >> 10;
  }

  int16_t mibi_celcius_{0};
};

static_assert(sizeof(Temperature) <= 2,
              "Temperature should be small enough to pass by value");
