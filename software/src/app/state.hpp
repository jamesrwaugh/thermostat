#pragma once

#include <stdint.h>

#include "event.hpp"

namespace State {

enum class Type : uint8_t {
  Started = 0,
  Idle,
  Heating,
  Cooling,
  ProgramTemp,
  ProgramDate,
  ProgramTime,
  ProgramAutoTimeSelect,
  ProgramAutoTimeDates,
  ProgramAutoTimeStart,
  ProgramAutoTimeEnd,
  ProgramAutoTimeTemps,
  NO_CHANGE,
};

struct Base {
  Base(State::Type stateId) : StateId(stateId) {}
  virtual State::Type handle_event(const Event::Base& event) = 0;
  virtual ~Base() = default;
  const State::Type StateId;
};

}  // namespace State
