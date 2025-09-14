#pragma once

#include <driver_rs_wrapper.hpp>

namespace Event {

struct Base {
  Base(uint8_t id) : id_(id) {}
  const uint8_t id_;
};

struct Type {
  enum TheType : uint8_t {
    UpButtonPressed = 0,
    DownButtonPressed,
    SelectButtonPressed,
    SecondPassed,
    FanModeChanged,
    HeatModeChanged,
    ReverseValveModeChanged
  };
};

struct UpButtonPressed : Base {
  UpButtonPressed() : Base(Event::Type::UpButtonPressed) {}
};

struct DownButtonPressed : Base {
  DownButtonPressed() : Base(Event::Type::DownButtonPressed) {}
};

struct SelectButtonPressed : Base {
  SelectButtonPressed() : Base(Event::Type::SelectButtonPressed) {}
};

struct SecondPassed : Base {
  SecondPassed() : Base(Event::Type::SecondPassed) {}
};

struct FanModeChanged : Base {
  FanModeChanged(FanModeT mode)
      : Base(Event::Type::FanModeChanged), Mode(mode) {}
  const FanModeT Mode;
};

struct HeatModeChanged : Base {
  HeatModeChanged(HeatModeT mode)
      : Base(Event::Type::HeatModeChanged), Mode(mode) {}
  const HeatModeT Mode;
};

struct ReverseValveModeChanged : Base {
  ReverseValveModeChanged(ReverseValveModeT mode)
      : Base(Event::Type::ReverseValveModeChanged), Mode(mode) {}
  const ReverseValveModeT Mode;
};

}  // namespace Event
