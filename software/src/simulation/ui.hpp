#pragma once

#include <ftxui/component/component_base.hpp>

#include "sim_thermostat.hpp"

class Ui : public ftxui::ComponentBase {
 public:
  Ui(SimAvrThermostat& thermostat);

 private:
  SimAvrThermostat& Thermostat_;
  int fan_selection_ = 0;   // 0 = Auto, 1 = On
  int heat_selection_ = 0;  // 0 = Heat, 1 = Cool, 2 = None
};