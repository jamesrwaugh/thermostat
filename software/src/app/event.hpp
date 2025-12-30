#pragma once

#include <driver_rs_wrapper.hpp>

namespace Event {

enum class Type : uint8_t {
  UpButtonPressed = 0,
  DownButtonPressed,
  SelectButtonPressed,
  HalfSecondPassed,
  SecondPassed,
  FanModeChanged,
  HeatModeChanged,
  ReverseValveModeChanged
};

struct Base {
  Base(Type id) : id_(id) {}
  const Type id_;
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

struct HalfSecondPassed : Base {
  HalfSecondPassed() : Base(Event::Type::HalfSecondPassed) {}
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
