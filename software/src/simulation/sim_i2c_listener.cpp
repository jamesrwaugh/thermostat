#include "sim_i2c_listener.hpp"

#include <simavr-toolbox/sim_base.hpp>
#include <string_view>

void FakeI2cCb2(struct avr_irq_t* irq, uint32_t value, void* param) {
  avr_twi_msg_irq_t msg;
  msg.u.v = value;
  auto cb = (I2cStructMessageCallback*)param;
  (*cb)(&msg);
}

SimI2CListener::SimI2CListener(avr_t* avr) {
  OnMessageFromAvr_ =
      std::bind(&SimI2CListener::OnMessageFromAvr, this, std::placeholders::_1);

  avr_irq_register_notify(
      avr_io_getirq(avr, AVR_IOCTL_TWI_GETIRQ(0), TWI_IRQ_OUTPUT), FakeI2cCb2,
      &OnMessageFromAvr_);

  OnMessageToAvr_ =
      std::bind(&SimI2CListener::OnMessageToAvr, this, std::placeholders::_1);

  avr_irq_register_notify(
      avr_io_getirq(avr, AVR_IOCTL_TWI_GETIRQ(0), TWI_IRQ_INPUT), FakeI2cCb2,
      &OnMessageToAvr_);
}

std::string_view GetMessageType(const avr_twi_msg_irq_t* value) {
  switch (value->u.twi.msg) {
    case TWI_COND_START:
      return "START";
    case TWI_COND_ADDR:
      return "ADDR";
    case TWI_COND_ACK:
      return "ACK";
    case TWI_COND_STOP:
      return "STOP";
    case TWI_COND_WRITE:
      return "WRITE";
    case TWI_COND_READ:
      return "READ";
    default:
      return "UNKNOWN";
  }
}

void SimI2CListener::OnMessageFromAvr(avr_twi_msg_irq_t* value) {
  sim_debug_log("I2C >> Addr: 0x%0x, R/W: %s, D: 0x%0x, Msg: %s",
                value->u.twi.addr >> 1, value->u.twi.addr & 1 ? "R" : "W",
                value->u.twi.data, GetMessageType(value).data());
}

void SimI2CListener::OnMessageToAvr(avr_twi_msg_irq_t* value) {
  sim_debug_log("I2C << Addr: 0x%0x, R/W: %s, D: 0x%0x, Msg: %s",
                value->u.twi.addr >> 1, value->u.twi.addr & 1 ? "R" : "W",
                value->u.twi.addr, value->u.twi.data,
                GetMessageType(value).data());
}