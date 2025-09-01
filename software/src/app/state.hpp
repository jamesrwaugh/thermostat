#pragma once

#include <stdint.h>

namespace State {
struct Type {
  enum TheType : uint8_t {
    RunningParent = 0,
    Idle,
    Heating,
    Cooling,
    COUNT,
  };
};
}  // namespace State
