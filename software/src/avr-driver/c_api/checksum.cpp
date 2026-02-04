#include "checksum.hpp"

uint8_t checksum(const uint8_t* data, uint16_t size) {
  uint32_t sum = 0;
  for (uint16_t i = 0; i < size; ++i) {
    sum += data[i];
  }
  return sum & 0xFF;
}
