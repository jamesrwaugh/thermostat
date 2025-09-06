#pragma once

#include <cstdint>
#include <simavr-toolbox/sim_i2c_base.hpp>
#include <vector>

class SimAvrI2CSmarterComponent : public SimAvrI2CComponent {
 public:
  SimAvrI2CSmarterComponent(avr_t* avr, uint8_t i2cAddress);

  void HandleI2CMessage(const avr_twi_msg_t& msg) override;
  void ResetStateMachine() override;
  virtual void OnDataReceived(const std::vector<uint8_t>& data) = 0;

 private:
  std::vector<uint8_t> DataBuffer_;
};