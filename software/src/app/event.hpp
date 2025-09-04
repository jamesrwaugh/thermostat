#pragma once

#include <etl/message.h>

#include <driver_rs_wrapper.hpp>

namespace Event {

struct Type {
  enum TheType : etl::message_id_t {
    UpButtonPressed = 0,
    DownButtonPressed,
    SelectButtonPressed,
    SecondPassed,
    FanModeChanged,
    HeatModeChanged,
    ReverseValveModeChanged
  };
};

struct UpButtonPressed : etl::message<Event::Type::UpButtonPressed> {};
struct DownButtonPressed : etl::message<Event::Type::DownButtonPressed> {};
struct SelectButtonPressed : etl::message<Event::Type::SelectButtonPressed> {};
struct SecondPassed : etl::message<Event::Type::SecondPassed> {};

struct FanModeChanged : etl::message<Event::Type::FanModeChanged> {
  FanModeChanged(FanModeT mode) : Mode(mode) {}
  const FanModeT Mode;
};

struct HeatModeChanged : etl::message<Event::Type::HeatModeChanged> {
  HeatModeChanged(HeatModeT mode) : Mode(mode) {}
  const HeatModeT Mode;
};

struct ReverseValveModeChanged
    : etl::message<Event::Type::ReverseValveModeChanged> {
  ReverseValveModeChanged(ReverseValveModeT mode) : Mode(mode) {}
  const ReverseValveModeT Mode;
};

}  // namespace Event
