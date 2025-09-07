#pragma once

#include "sim_i2c_smarter_base.hpp"

class SimFakeGu7000 : public SimAvrI2CSmarterComponent {
 public:
  enum class CoolStateE {
    Idle,
    Cooling,
    Heating,
  };

  struct State {
    CoolStateE CoolState{CoolStateE::Idle};
    uint8_t SetPoint{0};
    uint8_t Temp{0};
  };

  SimFakeGu7000(avr_t* avr);
  void OnDataReceived(const std::vector<uint8_t>& data) override;
  const State& GetState() const;

 private:
  State State_;
};