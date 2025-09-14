#pragma once

#include <stdint.h>

#include "event.hpp"

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

struct Base {
  virtual State::Type::TheType handle_event(const Event::Base& event) = 0;
};

}  // namespace State
