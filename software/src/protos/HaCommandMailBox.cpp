#include "HaCommandMailBox.hpp"

#include "checksum.hpp"

bool HaCommandMailBox::ReceiveByte(uint8_t byte, HaCommand& c) {
  buffer_[current_bytes_++] = byte;

  if (current_bytes_ >= sizeof(buffer_)) {
    DecodeHaCommand(&c, buffer_);
    uint8_t checksum_now = checksum(buffer_ + 1, sizeof(buffer_) - 1);
    current_bytes_ = 0;
    if (checksum_now == c.checksum) {
      return true;
    } else {
      c.checksum = 0xFF;
    }
  }

  return false;
}
