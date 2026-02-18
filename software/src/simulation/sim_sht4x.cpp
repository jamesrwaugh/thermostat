#include "sim_sht4x.hpp"

SimSHT4x::SimSHT4x(avr_t* avr) : SimAvrI2CComponent(avr, 0x44) {
  reading_buffer_.fill(0);
}

void SimSHT4x::HandleI2CMessage(const avr_twi_msg_t& msg) {
  if (msg.msg & TWI_COND_START) {
    SendToAvrI2CAck();
  } else if (msg.msg & TWI_COND_STOP) {
    ResetStateMachine();
  } else if (msg.msg & TWI_COND_WRITE) {
    SendToAvrI2CAck();
    switch (msg.data) {
      case 0xFD:
        state_ = State::SendReading;
        ReadTemperatureToBuffer();
        break;
    }
  } else if (msg.msg & TWI_COND_READ) {
    if (state_ == State::SendReading) {
      SendByteToAvrI2c(reading_buffer_.at(current_i2c_byte_++));
    }
  }
}

void SimSHT4x::SimulateTempChange(const RelayState& relays) {
  auto now = std::chrono::steady_clock::now();

  auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - last_temp_update_);

  if (delta > std::chrono::seconds(1)) {
    last_temp_update_ = now;

    if (relays.Heat) {
      celcius_ += 0.10;
    } else if (relays.Compressor) {
      celcius_ -= 0.10;
      percent_humidity_ += 0.10;
    } else {
      celcius_ += 0.025;
      percent_humidity_ += 0.05;
    }

    percent_humidity_ = std::min(percent_humidity_, MaxHumidity);
    celcius_ = std::min(celcius_, MaxTempC - 1);
  }
}

void SimSHT4x::ResetStateMachine() {
  state_ = State::None;
  current_i2c_byte_ = 0;
}

uint16_t SimSHT4x::CelsiusToTicks() const {
  return (65535.0 / 175) * (celcius_ + 45);
}

uint16_t SimSHT4x::HumidityToTicks() const {
  return (65535.0 / 125) * (percent_humidity_ + 6);
}

void SimSHT4x::ReadTemperatureToBuffer() {
  uint16_t temp = CelsiusToTicks();
  uint16_t hum = HumidityToTicks();
  reading_buffer_[0] = temp >> 8;
  reading_buffer_[1] = temp & 0xFF;
  reading_buffer_[2] = 0;  // CRC - TODO
  reading_buffer_[3] = hum >> 8;
  reading_buffer_[4] = hum & 0xFF;
  reading_buffer_[5] = 0;  // CRC - TODO
}
