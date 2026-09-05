#pragma once

#include <driver_rs_wrapper.hpp>

namespace Event {

enum class Type : uint8_t {
  UpButtonPressed = 0,
  DownButtonPressed,
  SelectButtonPressed,
  FanButtonPushed,
  HeatButtonPushed,
  TenMillisecondsPassed,
  HalfSecondPassed,
  SecondPassed,
  MqttConnected,
  MqttDisconnected,
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

struct HeatButtonPressed : Base {
  HeatButtonPressed() : Base(Event::Type::HeatButtonPushed) {}
};

struct FanButtonPressed : Base {
  FanButtonPressed() : Base(Event::Type::FanButtonPushed) {}
};

struct TenMillisecondsPassed : Base {
  TenMillisecondsPassed() : Base(Event::Type::TenMillisecondsPassed) {}
};

struct HalfSecondPassed : Base {
  HalfSecondPassed() : Base(Event::Type::HalfSecondPassed) {}
};

struct SecondPassed : Base {
  SecondPassed() : Base(Event::Type::SecondPassed) {}
};

struct MqttConnected : Base {
  MqttConnected() : Base(Event::Type::MqttConnected) {}
};

struct MqttDisconnected : Base {
  MqttDisconnected() : Base(Event::Type::MqttDisconnected) {}
};

}  // namespace Event
