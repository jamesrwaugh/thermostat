#include "coolable_parent.hpp"

#include <etl/placement_new.h>
#include <sys/types.h>

#include "HomeAssistantSerial.hpp"
#include "event.hpp"
#include "machine.hpp"
#include "state.hpp"

// ===================================================================== //

CoolableParent::CoolableParent(Machine& machine, State::Type stateId)
    : State::Base(stateId), machine_(machine) {
  DriverDisplayClearScreen();
  machine_.DisplaySetPointAndTemp();
}

CoolableParent::~CoolableParent() {}

State::Type CoolableParent::handle_event(const Event::Base& event) {
  switch (event.id_) {
    case Event::Type::UpButtonPressed: {
      return ChangeSetPoint(1);
    }
    case Event::Type::DownButtonPressed: {
      return ChangeSetPoint(-1);
    }
    case Event::Type::SelectButtonPressed: {
      return State::Type::ProgramTemp;
    }
    case Event::Type::SecondPassed: {
      machine_.ReadTemperatureAndReportIfChanged();
      TickChangeCounter();
      return DetermineNextState();
    }
    case Event::Type::FanButtonPushed: {
      const auto fanMode = machine_.SafeSaveState().BumpFanMode();

      if (fanMode == FanModeT::On) {
        DriverRelayOn(Relay::Fan);
        if (IsIdle()) {
          machine_.WriteHaActionStateTopicResponse(HaActionKey::Fan);
        }
      } else if (fanMode != FanModeT::On && IsIdle()) {
        DriverRelayOff(Relay::Fan);
        machine_.WriteHaActionStateTopicResponse(HaActionKey::Idle);
      }

      return State::Type::NO_CHANGE;
    }
    case Event::Type::HeatButtonPushed: {
      machine_.SafeSaveState().BumpHeatingMode();
      machine_.WriteHaModeStateTopicResponse();
      return DetermineNextState();
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

void CoolableParent::ActivateCoolingRelays(Relay onRelay, Relay offRelay,
                                           ReverseValveModeT onIfType) {
  if (machine_.ButtonState().ReverseValveState == onIfType) {
    DriverRelayOn(Relay::ReversingValve);
  } else {
    DriverRelayOff(Relay::ReversingValve);
  }

  DriverRelayOn(onRelay);
  DriverRelayOff(offRelay);
}

void CoolableParent::EnterHeatingOrCooling(HeatModeT mode) {
  if (machine_.SafeSaveState().FanMode() == FanModeT::Auto) {
    DriverRelayOn(Relay::Fan);
  }

  if (mode == HeatModeT::Cooling) {
    DriverDisplayIsCooling();
    ActivateCoolingRelays(Relay::Compressor, Relay::Heat,
                          ReverseValveModeT::OnForCooling);
    machine_.WriteHaActionStateTopicResponse(HaActionKey::Cooling);
  } else {
    DriverDisplayIsHeating();
    ActivateCoolingRelays(Relay::Heat, Relay::Compressor,
                          ReverseValveModeT::OnForHeating);
    machine_.WriteHaActionStateTopicResponse(HaActionKey::Heating);
  }
}

void CoolableParent::ExitHeatingOrCooling() {
  DriverRelayOff(Relay::Heat);
  DriverRelayOff(Relay::Compressor);
  DriverRelayOff(Relay::ReversingValve);

  if (machine_.SafeSaveState().FanMode() == FanModeT::Auto) {
    DriverRelayOff(Relay::Fan);
    machine_.WriteHaActionStateTopicResponse(HaActionKey::Idle);
  } else {
    machine_.WriteHaActionStateTopicResponse(HaActionKey::Fan);
  }
}

void CoolableParent::TickChangeCounter() {
  if (ChData.StateChangeTimeoutSec < ChData.MaxStateChangeTimeoutSec) {
    ChData.StateChangeTimeoutSec += 1;
  }
}

[[nodiscard]] bool CoolableParent::HasChangeTimeoutPassed() const {
  return ChData.StateChangeTimeoutSec >= ChData.MaxStateChangeTimeoutSec;
}

[[nodiscard]] State::Type CoolableParent::ChangeSetPoint(int8_t change) {
  auto& saveData = machine_.SafeSaveState();

  auto& setPoint = saveData.Data.set_point;

  if (setPoint == 1 && change < 0) {
    return State::Type::NO_CHANGE;
  }

  if (setPoint == 100 && change > 0) {
    return State::Type::NO_CHANGE;
  }

  setPoint += change;

  DriverDisplaySetPoint(setPoint, saveData.TemperatureUnit());

  return DetermineNextState();
}

[[nodiscard]] State::Type CoolableParent::DetermineNextState() {
  if (!HasChangeTimeoutPassed()) {
    return State::Type::NO_CHANGE;
  }

  const auto& saveData = machine_.SafeSaveState();
  const auto heatMode = saveData.HeatMode();
  const auto setPoint = saveData.Data.set_point;
  uint8_t temp = machine_.LastReadTemerature();

  if (heatMode == HeatModeT::None) {
    return State::Type::Idle;
  }

  if (temp >= setPoint && heatMode == HeatModeT::Heating) {
    return State::Type::Idle;
  } else if (temp <= setPoint && heatMode == HeatModeT::Cooling) {
    return State::Type::Idle;
  } else if (temp <= setPoint && heatMode == HeatModeT::Heating) {
    return State::Type::Heating;
  } else if (temp >= setPoint && heatMode == HeatModeT::Cooling) {
    return State::Type::Cooling;
  }

  return State::Type::NO_CHANGE;
}

bool CoolableParent::IsHeatingOrCooling() const {
  return machine_.get_state_id() == State::Type::Heating ||
         machine_.get_state_id() == State::Type::Cooling;
}

bool CoolableParent::IsIdle() const {
  return !IsHeatingOrCooling();
}
