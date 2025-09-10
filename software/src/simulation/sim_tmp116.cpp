#include "sim_tmp116.hpp"

#include "relay.hpp"

SimTMP116::SimTMP116(avr_t* avr)
    : SimAvrI2CComponent(avr, 0x48),
      LastTempTick(std::chrono::steady_clock::now()) {
  //
}

void SimTMP116::SetTempC(uint16_t temp) {
  TempCInternal_ = temp << 7;
}

void SimTMP116::HandleI2CMessage(const avr_twi_msg_t& msg) {
  if (msg.msg & TWI_COND_START) {
    SendToAvrI2CAck();
  } else if (msg.msg & TWI_COND_STOP) {
    ResetStateMachine();
  } else if (msg.msg & TWI_COND_WRITE) {
    SendToAvrI2CAck();
  } else if (msg.msg & TWI_COND_READ) {
    if (ReadState_ == ReadState::HighByte) {
      SendByteToAvrI2c(TempCInternal_ >> 8);
      ReadState_ = ReadState::LowByte;
    } else if (ReadState_ == ReadState::LowByte) {
      SendByteToAvrI2c(TempCInternal_ & 0xFF);
      ReadState_ = ReadState::OhNo;
    } else {
      std::abort();
    }
  }
}

void SimTMP116::SimulateTempChange(const RelayState& relays) {
  auto now = std::chrono::steady_clock::now();
  auto delta =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - LastTempTick);

  if (delta > std::chrono::seconds(1)) {
    LastTempTick = now;
    UpdateTemperature(relays);
  }
}

void SimTMP116::UpdateTemperature(const RelayState& relays) {
  if (relays.Heat) {
    // Heating temperature increases over time
    TempCInternal_ += 32;
  } else if (relays.Compressor) {
    // Cooling temperature decreases over time
    TempCInternal_ -= 32;
  } else {
    // Ambient temperature increases over time
    TempCInternal_ += 8;
  }
}

void SimTMP116::ResetStateMachine() {
  ReadState_ = ReadState::HighByte;
}
