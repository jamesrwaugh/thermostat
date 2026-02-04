#pragma once

#include "state.hpp"

class Started : State::Base {
 public:
  Started();
  virtual State::Type handle_event(const Event::Base& event);
};
