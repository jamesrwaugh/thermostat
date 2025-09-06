#pragma once

#include <ftxui/component/component_base.hpp>

#include "sim_thermostat.hpp"

class Ui : public ftxui::ComponentBase {
 public:
  Ui(SimAvrThermostat& thermostat);

 private:
  SimAvrThermostat& Thermostat_;
};