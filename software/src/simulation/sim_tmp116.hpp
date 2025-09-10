#pragma once

#include <chrono>
#include <simavr-toolbox/sim_i2c_base.hpp>

#include "relay.hpp"

class SimTMP116 final : public SimAvrI2CComponent {
 public:
  SimTMP116(avr_t* avr);

  void HandleI2CMessage(const avr_twi_msg_t& msg) override;
  void ResetStateMachine() override;

  void SetTempF(uint8_t tempF);
  void SimulateTempChange(const RelayState& relays);

 private:
  void UpdateTemperature(const RelayState& relays);
  float TempF_{78};
  std::chrono::steady_clock::time_point LastTempTick;
};