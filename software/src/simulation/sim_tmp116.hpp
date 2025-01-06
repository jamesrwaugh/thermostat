#pragma once

#include <simavr-toolbox/sim_i2c_base.hpp>

class SimTMP116 final : public SimAvrI2CComponent {
 public:
  void HandleI2CMessage(const avr_twi_msg_t &msg) override;
  void ResetStateMachine() override;
};