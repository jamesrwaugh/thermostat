#pragma once

#include <simavr-toolbox/sim_base.hpp>
#include <simavr-toolbox/sim_bouncy_switch.hpp>
#include <simavr-toolbox/sim_gu7000.hpp>
#include <string_view>

#include "ftxui_simulated_avr.hpp"

class SimAvrThermostat : public FtxUiSimulatedAvr {
 public:
  SimAvrThermostat(std::string_view filename, bool gdb, TaskReceiver& receiver);

  void PushUpButton();
  void PushDownButton();
  void SwitchHeatingHeat();
  void SwitchHeatingCooling();
  void SwitchHeatingNone();
  void SwitchFanOn();
  void SwitchFanAuto();
  void SendSerialMessage(std::string_view message);

 private:
  TaskSender S_;
  std::unique_ptr<SimGu7000> Screen;
  std::unique_ptr<SimBouncySwitch> UpButton;
  std::unique_ptr<SimBouncySwitch> DownButton;
  std::unique_ptr<SimBouncySwitch> TempHeat;
  std::unique_ptr<SimBouncySwitch> TempCool;
  std::unique_ptr<SimBouncySwitch> FanIsOn;
};