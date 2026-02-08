#pragma once

#include <chrono>
#include <simavr-toolbox/sim_i2c_smarter_base.hpp>

#include "relay.hpp"

class SimSHT4x final : public SimAvrI2CComponent {
 public:
  SimSHT4x(avr_t* avr);
  void HandleI2CMessage(const avr_twi_msg_t& msg) override;
  void SimulateTempChange(const RelayState& relays);
  void ResetStateMachine() override;
  uint16_t CelsiusToTicks() const;
  uint16_t HumidityToTicks() const;
  void ReadTemperatureToBuffer();

  enum class State { None, SendReading = 0 };
  State state_{State::None};
  std::array<uint8_t, 6> reading_buffer_;
  uint8_t current_i2c_byte_{0};
  double celcius_{23.0};
  double percent_humidity_{10.0};
  std::chrono::steady_clock::time_point last_temp_update_;
};
