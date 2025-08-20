#pragma once

#include <etl/hfsm.h>

#include "event.hpp"

class Machine : public etl::hfsm {
 public:
  Machine() : etl::hfsm(0) {}
  uint8_t SetPoint{70};
  Event::FanModeT FanMode{Event::FanModeT::Auto};
  Event::HeatModeT HeatMode{Event::HeatModeT::None};
};