#include "sim_i2c_listener.hpp"

SimI2CListener::SimI2CListener(avr_t* avr)
    : SimAvrI2CComponent(avr, [](avr_twi_msg_t*) { return true; }) {}

void SimI2CListener::HandleI2CMessage(const avr_twi_msg_t& msg) {
  // Do nothing.
  if (msg.msg & TWI_COND_START) {
    MessageInProgress_ = std::nullopt;
  } else if (msg.msg & TWI_COND_STOP) {
    MessageInProgress_ = std::nullopt;
  } else if (msg.msg & TWI_COND_WRITE) {
    MessageInProgress_ = std::nullopt;
  } else if (msg.msg & TWI_COND_READ) {
    MessageInProgress_ = std::nullopt;
  }
}