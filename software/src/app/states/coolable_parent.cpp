#include "coolable_parent.hpp"

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
#include "machine.hpp"
#include "state.hpp"

CoolableParent::CoolableParent(Machine& machine, State::Type stateId)
    : State::Base(stateId), machine_(machine) {
  DriverDisplayClearScreen();
  machine_.DisplaySetPointAndTemp();
}

CoolableParent::~CoolableParent() {}

State::Type CoolableParent::handle_event(const Event::Base& event) {
  switch (event.id_) {
    case Event::Type::UpButtonPressed: {
      return machine_.ChangeSetPoint(1);
    }
    case Event::Type::DownButtonPressed: {
      return machine_.ChangeSetPoint(-1);
    }
    case Event::Type::SelectButtonPressed: {
      return State::Type::ProgramTemp;
    }
    case Event::Type::SecondPassed: {
      machine_.ReadTemperatureAndDisplayIfChanged();
      machine_.TickChangeCounter();
      return machine_.DetermineNextState();
    }
    case Event::Type::FanButtonPushed: {
      const auto fanMode = machine_.SafeSaveState().BumpFanMode();

      if (fanMode == FanModeT::On) {
        DriverRelayOn(Relay::Fan);
      } else if (fanMode == FanModeT::Auto &&
                 !machine_.IsHeatingOrCoolingNow()) {
        DriverRelayOff(Relay::Fan);
      }

      return State::Type::NO_CHANGE;
    }
    case Event::Type::HeatButtonPushed: {
      machine_.SafeSaveState().BumpHeatingMode();
      return machine_.DetermineNextState();
    }
    case Event::Type::ReverseValveModeChanged: {
      const auto& valveEvent =
          static_cast<const Event::ReverseValveModeChanged&>(event);
      machine_.ButtonState().ReverseValveState = valveEvent.Mode;
      return State::Type::Idle;
    }
    default:
      return State::Type::NO_CHANGE;
  }
}