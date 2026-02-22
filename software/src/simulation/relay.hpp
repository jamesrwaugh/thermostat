#pragma once

#include <stdint.h>

enum class RelayType : uint8_t {
  Fan = 0,
  Compressor = 1,
  Heat = 2,
  ReverseValve = 3,
};

struct RelayState {
  bool Compressor{false};
  bool Fan{false};
  bool Heat{false};
  bool ReverseValve{false};
};