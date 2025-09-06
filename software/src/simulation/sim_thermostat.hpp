#pragma once

#include <simavr-toolbox/sim_base.hpp>
#include <simavr-toolbox/sim_bouncy_switch.hpp>
#include <simavr-toolbox/sim_gu7000.hpp>
#include <string_view>

#include "ftxui_simulated_avr.hpp"
#include "relay.hpp"
#include "sim_tmp116.hpp"

extern "C" {
#include <simavr/parts/ds1338_virt.h>
}

class SimAvrThermostat : public FtxUiSimulatedAvr {
 public:
  SimAvrThermostat(std::string_view filename, bool gdb, TaskReceiver& receiver);

  using RelayCb = std::function<void(Relay r, bool value)>;

  virtual void BeforeAvrCycleSideEffect() override;

  void PushUpButton();
  void PushDownButton();
  void PushSelectButton();
  void SwitchHeatingHeat();
  void SwitchHeatingCooling();
  void SwitchHeatingNone();
  void SwitchFanOn();
  void SwitchFanAuto();
  void SwitchReverseValve(bool onForHeat);
  void SendSerialMessage(std::string_view message);
  const RelayState& GetRelayState() const;

 private:
  void OnRelayChange(Relay r, bool value);

  TaskSender S_;
  std::unique_ptr<SimGu7000> Screen;
  std::unique_ptr<SimBouncySwitch> UpButton;
  std::unique_ptr<SimBouncySwitch> DownButton;
  std::unique_ptr<SimBouncySwitch> SelectButton;
  std::unique_ptr<SimBouncySwitch> ReverseValveSwitch;
  std::unique_ptr<SimBouncySwitch> TempHeat;
  std::unique_ptr<SimBouncySwitch> TempCool;
  std::unique_ptr<SimBouncySwitch> FanSwitch;
  ds1338_virt_t Rtc_;
  std::unique_ptr<SimTMP116> Tmp116_;
  RelayState Relays_;
  RelayCb RelayCb_;
};