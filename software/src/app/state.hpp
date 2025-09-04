#pragma once

#include <stdint.h>

namespace State {
struct Type {
  enum TheType : uint8_t {
    CoolableParent = 0,
    Idle,
    Heating,
    Cooling,
    Program,
    COUNT,
  };
};
}  // namespace State
