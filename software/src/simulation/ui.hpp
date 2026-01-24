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
};
