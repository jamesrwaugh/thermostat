#pragma once

#include <limits.h>
#include <stdint.h>

#include "thermo_data_types.hpp"

// ==================================================== //

void Clamp(auto& value, auto min, auto max) {
  if (value > max) {
    value = max;
  } else if (value < min) {
    value = min;
  }
}

// ==================================================== //

class Humidity {
 public:
  static constexpr uint16_t MibiFactor = 512;
  static constexpr uint16_t MinValue = 0;
  static constexpr uint16_t MaxValue = (99u * MibiFactor);

  typedef uint8_t WholeType;
  typedef uint16_t MibiType;

  Humidity& SetFromSht4xSensor(uint16_t device_ticks) {
    uint32_t value = ((8000 * (uint32_t)device_ticks) >> 13) - 2560;
    ::Clamp(value, MinValue, MaxValue);
    mibi_percent_ = value;
    return *this;
  }

  WholeType ToWholePercent() const {
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
  void Clamp() {
    if (mibi_percent_ > MaxValue) {
      mibi_percent_ = MaxValue;
    } else if (mibi_percent_ < MinValue) {
      mibi_percent_ = MinValue;
    }
  }

  // Percent Humidity * 512.
  // This allows us to store fractional humidity as well
  // as only require bitshifts to encode and decode whole values,
  // to save on flash size, instead of divide by 1000 for example.
  MibiType mibi_percent_{0};
};

static_assert(sizeof(Humidity) <= 2,
              "Humidity should be small enough to pass by value");

// ==================================================== //

class Temperature {
 public:
  static constexpr uint16_t MibiFactor = 256;
  static constexpr uint16_t MibiWholePartMask = 0xFF00;
  static constexpr uint16_t MibiOneHalfDegree = (MibiFactor / 2);
  static constexpr uint16_t MibiOneForthDegree = (MibiFactor / 4);
  static constexpr uint16_t MibiOneEighthDegree = (MibiFactor / 8);
  static constexpr uint16_t MibiOneSixteenthDegree = (MibiFactor / 16);
  static constexpr uint16_t MibiOneTwentyEighthDegree = (MibiFactor / 128);
  static constexpr uint16_t MibiThreeEighthsDegrees = 3 * MibiOneEighthDegree;
  static constexpr uint16_t MibiCToFFactor =
      (MibiOneHalfDegree + MibiOneSixteenthDegree -
       MibiOneTwentyEighthDegree);  //  0.5546875 - Approximates 5/9
  static constexpr int8_t MinCelciusValue = -63;
  static constexpr int8_t MaxCelciusValue = 127;
  static constexpr int8_t MinFahrenheitValue = -81;
  static constexpr int16_t MaxFahrenheitValue = 260;
  static constexpr int16_t MaxMibiValue = (MaxCelciusValue * MibiFactor);
  static constexpr int16_t MinMibiValue = (MinCelciusValue * MibiFactor);

  static_assert(
      MinMibiValue - INT16_MIN >= MibiFactor,
      "Leave some space around MinMibiValue to account for any off-by-ones");

  static_assert(
      INT16_MAX - MaxMibiValue >= (MibiFactor - 1),
      "Leave some space around MaxMibiValue to account for any off-by-ones");

  static_assert(MaxMibiValue > 0, "Error setting max value");
  static_assert(MinMibiValue < 0, "Error setting min value");
  static_assert(MaxMibiValue > MinMibiValue, "Error setting values");

  typedef int16_t WholeType;
  typedef int16_t MibiType;
  typedef int32_t MibiInConvertType;

  [[nodiscard]] static Temperature FromCelcius(WholeType celcius) {
    Temperature t;
    t.SetFromCelcius(celcius);
    return t;
  }

  [[nodiscard]] static Temperature FromFahrenheit(WholeType fahrenheit) {
    Temperature t;
    t.SetFromFahrenheit(fahrenheit);
    return t;
  }

  [[nodiscard]] static Temperature FromMibiCelcius(MibiType mibicelcius) {
    Temperature t;
    t.SetFromMibiCelcius(mibicelcius);
    return t;
  }

  Temperature& SetFromTemperature(const Temperature& t) {
    mibi_celcius_ = t.GetMibiCelcius();
    Clamp();
    return *this;
  }

  Temperature& SetFromMibiCelcius(MibiType mibi_celcius) {
    mibi_celcius_ = mibi_celcius;
    Clamp();
    return *this;
  }

  Temperature& SetFromCelcius(WholeType celcius) {
    ::Clamp(celcius, MinCelciusValue, MaxCelciusValue);
    mibi_celcius_ = celcius * MibiFactor;
    return *this;
  }

  Temperature& SetFromFahrenheit(WholeType fahrenheit) {
    ::Clamp(fahrenheit, MinFahrenheitValue, MaxFahrenheitValue);
    mibi_celcius_ = WholeFtoMibiC(fahrenheit);
    if (fahrenheit > 0) {
      mibi_celcius_ += MibiOneTwentyEighthDegree;
    } else {
      mibi_celcius_ -= MibiOneTwentyEighthDegree;
    }
    return *this;
  }

  Temperature& SetFromSht4xSensor(uint16_t device_ticks) {
    int32_t value = ((((int32_t)device_ticks) << 13) / 11983) - 11520;
    ::Clamp(value, MinMibiValue, MaxMibiValue);
    mibi_celcius_ = value;
    return *this;
  }

  Temperature& ChangeByMibiCelcius(MibiInConvertType amount, bool increment) {
    if (increment) {
      MibiType sum = 0;
      if (__builtin_add_overflow(mibi_celcius_, amount, &sum)) {
        mibi_celcius_ = MaxMibiValue;
      } else {
        mibi_celcius_ = sum;
      }
    } else {
      MibiType sum = 0;
      if (__builtin_sub_overflow(mibi_celcius_, amount, &sum)) {
        mibi_celcius_ = MinMibiValue;
      } else {
        mibi_celcius_ = sum;
      }
    }

    return *this;
  }

  __attribute__((noinline)) WholeType
  GetUnitWhole(TemperatureUnitT unit) const {
    return unit == TemperatureUnitT::Celsius ? GetCelciusWhole()
                                             : GetFahrenheitWhole();
  }

  __attribute__((noinline)) WholeType
  GetRoundedUnitWhole(TemperatureUnitT unit) const {
    auto mibi = (unit == TemperatureUnitT::Celsius) ? mibi_celcius_
                                                    : MibiCToF(mibi_celcius_);
    return GetRoundedMibi(mibi) / MibiFactor;
  }

  WholeType GetCelciusWhole() const {
    return mibi_celcius_ / MibiFactor;
  }

  WholeType GetFahrenheitWhole() const {
    auto x = MibiCToF(mibi_celcius_);
    return x / MibiFactor;
  }

  static MibiInConvertType MibiCToF(const MibiType& celcius) {
    MibiInConvertType mibi_fahrenheit = celcius;
    mibi_fahrenheit *= 9;
    mibi_fahrenheit /= 5;
    mibi_fahrenheit += (32u * MibiFactor);
    return mibi_fahrenheit;
  }

  static MibiType WholeFtoMibiC(const WholeType& fahrenheit) {
    int32_t mibi_celcius = fahrenheit;
    mibi_celcius *= MibiFactor;
    mibi_celcius -= (32u * MibiFactor);
    mibi_celcius *= 5;
    mibi_celcius /= 9;
    return mibi_celcius;
  }

  MibiType GetMibiCelcius() const {
    return mibi_celcius_;
  }

  Temperature& ChangeBy1Unit(TemperatureUnitT unit, bool increment) {
    return ChangeByUnit(unit, 1, increment);
  }

  Temperature& ChangeByUnit(TemperatureUnitT unit,
                            uint8_t amount,
                            bool increment) {
    unit == TemperatureUnitT::Celsius ? ChangeByDegC(amount, increment)
                                      : ChangeByDegF(amount, increment);
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
  static MibiInConvertType GetRoundedMibi(const MibiInConvertType& mibi) {
    if (mibi >= 0) {
      MibiInConvertType sum = 0;
      if (__builtin_add_overflow(
              mibi, mibi & MibiOneHalfDegree ? MibiFactor : 0, &sum)) {
        return MaxMibiValue;
      } else {
        return sum;
      }
    } else {
      MibiInConvertType whole =
          mibi &
          ((static_cast<MibiInConvertType>(0xFFFF) << 16) | MibiWholePartMask);
      if (mibi & MibiOneHalfDegree) {
        return whole + MibiFactor;
      } else {
        return whole;  // Chop off decimal
      }
    }
  }

  void Clamp() {
    ::Clamp(mibi_celcius_, MinMibiValue, MaxMibiValue);
  }

  void ChangeByDegC(uint8_t amount, bool increment) {
    ChangeByMibiCelcius(amount * MibiFactor, increment);
    Clamp();
  }

  void ChangeByDegF(uint8_t amount, bool increment) {
    ChangeByMibiCelcius(amount * MibiCToFFactor, increment);
    Clamp();
  }

  // Degrees Celsius * 256.
  // This allows us to store fractional temperatures as well
  // as only require bitshifts to encode and decode whole values,
  // to save on flash size, instead of divide by 1000 for example.
  MibiType mibi_celcius_{0};
};

static_assert(sizeof(Temperature) <= 2,
              "Temperature should be small enough to pass by value");

// ==================================================== //