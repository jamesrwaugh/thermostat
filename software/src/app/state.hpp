#pragma once

#include <stdint.h>

namespace State {
struct Type {
  enum TheType : uint8_t {
    RunningParent = 0,
    Idle = 1,
    Heating = 2,
    Cooling = 3,
  };
};
}  // namespace State
