#pragma once

#include <etl/alignment.h>
#include <stdint.h>

#include "../data_types.hpp"

class Humidity {
 public:
  static constexpr uint16_t MibiFactor = 1024;

  Humidity& SetFromSht4xSensor(uint16_t device_ticks) {
    mibi_percent_ = ((16000 * (int32_t)device_ticks) >> 13) - 6144;
    return *this;
  }

  uint8_t ToPercent() const {
    return mibi_percent_ / MibiFactor;
  }

  int8_t operator<=>(const Humidity& other) const {
    if (mibi_percent_ < other.mibi_percent_) return -1;
    if (mibi_percent_ > other.mibi_percent_) return 1;
    return 0;
  }

  bool operator==(const Humidity& other) const {
    return mibi_percent_ == other.mibi_percent_;
  }

 private:
  // Percent Humidity * 1024.
  // This allows us to store fractional humidity as well
  // as only require bitshifts to encode and decode whole values,
  // to save on flash size, instead of divide by 1000 for example.
  int16_t mibi_percent_{0};
};

static_assert(sizeof(Humidity) <= 2,
              "Humidity should be small enough to pass by value");

class Temperature {
 public:
  static constexpr uint16_t MibiFactor = 512;
  static constexpr uint16_t MibiOneHalfDegree = (MibiFactor / 2);
  static constexpr uint16_t MibiOneForthDegree = (MibiFactor / 4);
  static constexpr uint16_t MibiOneEighthDegree = (MibiFactor / 8);
  static constexpr uint16_t MibiOneSixteenthDegree = (MibiFactor / 16);
  static constexpr uint16_t MibiOneTwentyEightDegree = (MibiFactor / 128);
  static constexpr uint16_t MibiThreeEighthsDegrees = 3 * MibiOneEighthDegree;
  static constexpr uint16_t MibiCToFFactor =
    (MibiOneHalfDegree + MibiOneSixteenthDegree -
     MibiOneTwentyEightDegree);  //  0.5546875 - Approximates 5/9 (0.555556)
  static constexpr uint16_t MaxFahrenheit = 127;

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

  Temperature& SetFromTemperature(const Temperature& t) {
    mibi_celcius_ = t.GetMibiCelcius();
    return *this;
  }

  Temperature& SetFromMibiCelcius(uint16_t mibi_celcius) {
    mibi_celcius_ = mibi_celcius;
    return *this;
  }

  Temperature& SetFromCelcius(uint16_t celcius) {
    mibi_celcius_ = celcius * MibiFactor;
    return *this;
  }

  Temperature& SetFromSht4xSensor(uint16_t device_ticks) {
    mibi_celcius_ = ((11200 * (int32_t)device_ticks) >> 13) - 23040;
    return *this;
  }

  Temperature& ChangeByMibiCelcius(uint16_t amount, bool increment) {
    if (increment) {
      mibi_celcius_ += amount;
    } else {
      mibi_celcius_ -= amount;
    }
    return *this;
  }

  int8_t GetUnitWhole(TemperatureUnitT unit) const {
    return unit == TemperatureUnitT::Celsius ? GetCelciusWhole()
                                             : GetFahrenheitWhole();
  }

  int16_t GetMibiCelcius() const {
    return mibi_celcius_;
  }

  Temperature& ChangeBy1Unit(TemperatureUnitT unit, bool increment) {
    unit == TemperatureUnitT::Celsius ? ChangeBy1C(increment)
                                      : ChangeBy1F(increment);
    return *this;
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
    ChangeByMibiCelcius(MibiFactor, increment);
  }

  void ChangeBy1F(bool increment) {
    ChangeByMibiCelcius(MibiCToFFactor, increment);
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

  // Degrees Celsius * 512.
  // This allows us to store fractional temperatures as well
  // as only require bitshifts to encode and decode whole values,
  // to save on flash size, instead of divide by 1000 for example.
  int16_t mibi_celcius_{0};
};

static_assert(sizeof(Temperature) <= 2,
              "Temperature should be small enough to pass by value");
