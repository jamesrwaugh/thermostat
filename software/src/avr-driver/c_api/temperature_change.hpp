#pragma once

#include "temperature.hpp"

struct TemperatureChangeInfo {
  TemperatureChangeInfo() {}

  TemperatureChangeInfo(const Temperature& to,
                        const Temperature& tn,
                        const Humidity& ho,
                        const Humidity& hn)
      : OldTemperature{to},
        NewTemperature{tn},
        OldHumidity{ho},
        NewHumidity{hn} {}

  bool TemperatureChanged() {
    return OldTemperature.GetCelciusWhole() != NewTemperature.GetCelciusWhole();
  }

  bool HumidityChanged() {
    return OldHumidity.ToWholePercent() != NewHumidity.ToWholePercent();
  }

  const Temperature OldTemperature;
  const Temperature NewTemperature;
  const Humidity OldHumidity;
  const Humidity NewHumidity;
};