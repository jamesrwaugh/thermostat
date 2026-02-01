#pragma once

#include <simavr-toolbox/sim_47l04.h>

#include <ftxui-toolbox/ftxui_simulated_avr.hpp>
#include <simavr-toolbox/sim_base.hpp>
#include <simavr-toolbox/sim_bouncy_switch.hpp>
#include <simavr-toolbox/sim_gu7000.hpp>
#include <string_view>

#include "relay.hpp"
#include "sim_gu7000_i2c.hpp"
#include "sim_gu7000_real.hpp"
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
  void PushFanButton();
  void PushHeatButton();
  void SwitchReverseValve(bool onForHeat);
  void SendSerialMessage(std::string_view message);
  const RelayState& GetRelayState() const;
  const SimGu7000Real::DisplayMemory& GetScreenMemory() const;
  avr_t* GetAvr() const;

 private:
  void OnRelayChange(Relay r, bool value);

  TaskSender S_;
  std::unique_ptr<SimGu7000I2C> Screen;
  std::unique_ptr<SimBouncySwitch> UpButton;
  std::unique_ptr<SimBouncySwitch> DownButton;
  std::unique_ptr<SimBouncySwitch> SelectButton;
  std::unique_ptr<SimBouncySwitch> ReverseValveSwitch;
  std::unique_ptr<SimBouncySwitch> HeatButton;
  std::unique_ptr<SimBouncySwitch> FanButton;
  ds1338_virt_t Rtc_;
  std::unique_ptr<SimTMP116> Tmp116_;
  RelayState Relays_;
  RelayCb RelayCb_;
  std::unique_ptr<Sim47LXX> Eeprom_;

  std::chrono::steady_clock::time_point LastMsTick_;
};
