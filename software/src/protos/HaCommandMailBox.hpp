#pragma once

#include "HomeAssistantSerial_bp.h"

class HaCommandMailBox {
 public:
  static_assert(BYTES_LENGTH_HA_COMMAND > 1,
                "HaCommand must be greater than 1");

  bool ReceiveByte(uint8_t byte, HaCommand& c);

 private:
  uint8_t current_bytes_{0};
  uint8_t buffer_[BYTES_LENGTH_HA_COMMAND];
};
