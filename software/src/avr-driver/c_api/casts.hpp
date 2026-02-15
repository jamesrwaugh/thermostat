#pragma once

#include <stdint.h>

inline constexpr uint8_t u8(auto x) {
  return static_cast<uint8_t>(x);
}

inline constexpr uint16_t u16(auto x) {
  return static_cast<uint16_t>(x);
}
