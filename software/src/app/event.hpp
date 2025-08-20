#pragma once

#include <etl/message.h>

namespace Event {

struct Type {
  enum TheType : etl::message_id_t {
    UpButtonPressed = 0,
    DownButtonPressed = 1,
    SecondPassed = 2,
    FanModeChanged = 3,
    HeatModeChanged = 4,
  };
};

enum class FanModeT : uint8_t { On, Off, Auto };
enum class HeatModeT : uint8_t { Heating, Cooling, None };

struct UpButtonPressed : etl::message<Event::Type::UpButtonPressed> {};
struct DownButtonPressed : etl::message<Event::Type::DownButtonPressed> {};
struct SecondPassed : etl::message<Event::Type::SecondPassed> {};

struct FanModeChanged : etl::message<Event::Type::FanModeChanged> {
  FanModeChanged(FanModeT mode) : Mode(mode) {}
  const FanModeT Mode;
};

struct HeatModeChanged : etl::message<Event::Type::HeatModeChanged> {
  HeatModeChanged(HeatModeT mode) : Mode(mode) {}
  const HeatModeT Mode;
};

}  // namespace Event
