#include "sim_i2c_smarter_base.hpp"

SimAvrI2CSmarterComponent::SimAvrI2CSmarterComponent(avr_t* avr,
                                                     uint8_t i2cAddress)
    : SimAvrI2CComponent(avr, i2cAddress) {}

void SimAvrI2CSmarterComponent::HandleI2CMessage(const avr_twi_msg_t& msg) {
  if (msg.msg & TWI_COND_START) {
    ResetStateMachine();
    SendToAvrI2CAck();
  } else if (msg.msg & TWI_COND_STOP) {
    OnDataReceived(DataBuffer_);
    ResetStateMachine();
    SendToAvrI2CAck();
  } else if (msg.msg & TWI_COND_WRITE) {
    DataBuffer_.push_back(msg.data);
    SendToAvrI2CAck();
  } else if (msg.msg & TWI_COND_READ) {
    // No ack
  }
}

void SimAvrI2CSmarterComponent::ResetStateMachine() {
  DataBuffer_.clear();
}