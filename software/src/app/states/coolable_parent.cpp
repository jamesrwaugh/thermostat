#include "coolable_parent.hpp"

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
#include "state.hpp"

CoolableParent::CoolableParent(Machine& machine) : machine_(machine) {
  machine_.ResetStateChangeData();

  machine_.ReadAndApplySettings();

  DriverDisplaySetPoint(machine_.SaveState().set_point,
                        machine_.SafeSaveState().TemperatureUnit());

  machine_.ReadTemperature();
}

CoolableParent::~CoolableParent() {
  DriverRelayOff(Relay::Fan);
  machine_.ResetStateChangeData();
}

State::Type::TheType CoolableParent::handle_event(const Event::Base& event) {
  switch (event.id_) {
    case Event::Type::UpButtonPressed: {
      return machine_.ChangeSetPoint(1);
    }
    case Event::Type::DownButtonPressed: {
      return machine_.ChangeSetPoint(-1);
    }
    case Event::Type::SelectButtonPressed: {
      return State::Type::Program;
    }
    case Event::Type::SecondPassed: {
      machine_.ReadTemperature();
      machine_.TickChangeCounter();
      return machine_.DetermineNextState();
    }
    case Event::Type::FanModeChanged: {
      const auto& fanEvent = static_cast<const Event::FanModeChanged&>(event);
      machine_.ButtonState().FanState = fanEvent.Mode;

      if (fanEvent.Mode == FanModeT::On) {
        DriverRelayOn(Relay::Fan);
      } else if (fanEvent.Mode == FanModeT::Auto &&
                 !machine_.IsHeatingOrCoolingNow()) {
        DriverRelayOff(Relay::Fan);
      }

      return State::Type::CoolableParent;  // Stay in current state
    }
    case Event::Type::HeatModeChanged: {
      const auto& heatEvent = static_cast<const Event::HeatModeChanged&>(event);
      machine_.ButtonState().HeatingState = heatEvent.Mode;
      return machine_.DetermineNextState();
    }
    case Event::Type::ReverseValveModeChanged: {
      const auto& valveEvent =
          static_cast<const Event::ReverseValveModeChanged&>(event);
      machine_.ButtonState().ReverseValveState = valveEvent.Mode;
      return State::Type::Idle;
    }
    default:
      return State::Type::CoolableParent;  // Stay in current state
  }
}