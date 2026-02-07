#include "coolable_parent.hpp"

#include <etl/placement_new.h>
#include <sys/types.h>

#include "HomeAssistantSerial.hpp"
#include "event.hpp"
#include "machine.hpp"
#include "state.hpp"
#include "temperature.hpp"

// ===================================================================== //

CoolableParent::CoolableParent(Machine& machine, State::Type stateId,
                               const Image* const a, const Image* const b)
    : State::Base(stateId),
      machine_(machine),
      status_image_a_{a},
      status_image_b_{b} {
  DriverDisplayClearScreen();
  machine_.DisplaySetPointAndTemp();
}

CoolableParent::~CoolableParent() {}

State::Type CoolableParent::handle_event(const Event::Base& event) {
  switch (event.id_) {
    case Event::Type::UpButtonPressed: {
      return ChangeSetPoint(true);
    }
    case Event::Type::DownButtonPressed: {
      return ChangeSetPoint(false);
    }
    case Event::Type::SelectButtonPressed: {
      return State::Type::ProgramTemp;
    }
    case Event::Type::SecondPassed: {
      machine_.ReadTemperatureAndReportIfChanged();
      if (status_image_a_ && status_image_b_) {
        image_state_ = !image_state_;
        DrawImage(68, true, image_state_ ? *status_image_a_ : *status_image_b_);
      }
      return DetermineNextState();
    }
    case Event::Type::FanButtonPushed: {
      const auto fanMode = machine_.SaveState().BumpFanMode();

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
      machine_.SaveState().BumpHeatingMode();
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
  if (machine_.SaveState().FanMode() == FanModeT::Auto) {
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

  if (machine_.SaveState().FanMode() == FanModeT::Auto) {
    DriverRelayOff(Relay::Fan);
    machine_.WriteHaActionStateTopicResponse(HaActionKey::Idle);
  } else {
    machine_.WriteHaActionStateTopicResponse(HaActionKey::Fan);
  }
}

[[nodiscard]] State::Type CoolableParent::ChangeSetPoint(bool increment) {
  auto& saveData = machine_.SaveState();

  auto setPoint = saveData.SetPoint();
  setPoint.ChangeBy1Unit(saveData.TemperatureUnit(), increment);
  saveData.SetSetPoint(setPoint);

  DriverDisplaySetPoint(setPoint, saveData.TemperatureUnit());

  return DetermineNextState();
}

[[nodiscard]] State::Type CoolableParent::DetermineNextState() {
  const auto& saveData = machine_.SaveState();
  const auto heatMode = saveData.HeatMode();
  const auto setPoint = saveData.SetPoint();
  const auto temp = machine_.CurrentTemperature();

  if (heatMode == HeatModeT::None) {
    return State::Type::Idle;
  }

  Temperature upperLimit;
  Temperature lowerLimit;

  upperLimit  //
    .SetFromTemperature(setPoint)
    .ChangeByMibiCelcius(Temperature::MibiThreeEighthsDegrees, true);

  lowerLimit  //
    .SetFromTemperature(setPoint)
    .ChangeByMibiCelcius(Temperature::MibiThreeEighthsDegrees, false);

  if (temp >= upperLimit && heatMode == HeatModeT::Heating) {
    return State::Type::Idle;
  } else if (temp <= lowerLimit && heatMode == HeatModeT::Heating) {
    return State::Type::Heating;
  } else if (temp <= lowerLimit && heatMode == HeatModeT::Cooling) {
    return State::Type::Idle;
  } else if (temp >= upperLimit && heatMode == HeatModeT::Cooling) {
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
