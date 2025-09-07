#pragma once

#include <ftxui-toolbox/i2c_listener_renderer.hpp>
#include <ftxui-toolbox/locked_dequeue.hpp>
#include <ftxui/component/component_base.hpp>

#include "sim_thermostat.hpp"

class Ui : public ftxui::ComponentBase {
 public:
  Ui(SimAvrThermostat& thermostat, LockedDequeue& logs);

 private:
  SimAvrThermostat& Thermostat_;
  int fan_selection_ = 0;   // 0 = Auto, 1 = On
  int heat_selection_ = 0;  // 0 = Heat, 1 = Cool, 2 = None
};