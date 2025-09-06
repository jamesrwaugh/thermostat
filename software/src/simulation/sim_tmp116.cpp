#include "sim_tmp116.hpp"

SimTMP116::SimTMP116(avr_t* avr, uint8_t i2cAddress)
    : SimAvrI2CComponent(avr, i2cAddress),
      LastTempTick(std::chrono::steady_clock::now()) {
  //
}

void SimTMP116::SetTempF(uint8_t tempF) {
  TempF_ = tempF;
}

void SimTMP116::HandleI2CMessage(const avr_twi_msg_t& msg) {
  if (msg.msg & TWI_COND_START) {
    SendToAvrI2CAck();
  } else if (msg.msg & TWI_COND_STOP) {
    ResetStateMachine();
  } else if (msg.msg & TWI_COND_WRITE) {
    SendToAvrI2CAck();
  } else if (msg.msg & TWI_COND_READ) {
    SendByteToAvrI2c(TempF_);
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
    TempF_ += 0.01;
  } else if (relays.Compressor) {
    // Cooling temperature decreases over time
    TempF_ -= 0.01;
  } else {
    // Ambient temperature increases over time
    TempF_ += 0.001;
  }
}

void SimTMP116::ResetStateMachine() {}
