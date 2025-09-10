#pragma once

#include <chrono>
#include <simavr-toolbox/sim_i2c_base.hpp>

#include "relay.hpp"

class SimTMP116 final : public SimAvrI2CComponent {
 public:
  SimTMP116(avr_t* avr);

  void HandleI2CMessage(const avr_twi_msg_t& msg) override;
  void ResetStateMachine() override;

  void SetTempC(uint16_t temp);
  void SimulateTempChange(const RelayState& relays);

 private:
  enum class ReadState { LowByte, HighByte, OhNo };

  void UpdateTemperature(const RelayState& relays);
  uint16_t TempCInternal_{25 << 7};
  std::chrono::steady_clock::time_point LastTempTick;
  ReadState ReadState_{ReadState::HighByte};
};