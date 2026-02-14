#include "coolable_parent.hpp"

#include <Noritake_VFD_GU7000.h>
#include <etl/placement_new.h>
#include <sys/types.h>

#include "HomeAssistantSerial.hpp"
#include "event.hpp"
#include "machine.hpp"
#include "state.hpp"
#include "temperature.hpp"

// ===================================================================== //

CoolableParent::CoolableParent(Machine& machine, State::Type stateId)
    : State::Base(stateId),
      machine_(machine),
      rctx_{machine.GetRenderContext()} {}

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
    case Event::Type::TenMillisecondsPassed: {
      if (render_count_++ >= 5) {
        render_count_ = 0;
        Render();
      }
      return State::Type::NO_CHANGE;
    }
    case Event::Type::SecondPassed: {
      TemperatureChangeInfo info;
      machine_.ReadTemperatureAndReportIfChanged(info);
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

void CoolableParent::Render() {
  const auto& saveData = machine_.SaveState();

  if (!rctx_.temperature_manager_.IsFinished()) {
    rctx_.temperature_manager_.ScrollOnce();
    rctx_.renderer_.DrawTemperature(saveData.TemperatureUnit());
  }

  if (!rctx_.humidity_manager_.IsFinished()) {
    rctx_.humidity_manager_.ScrollOnce();
    rctx_.renderer_.DrawHumidity();
  }

  rctx_.renderer_.DrawHeatingStatus(saveData.HeatMode(), IsHeatingOrCooling());

  if (last_set_point_ != saveData.SetPoint()) {
    last_set_point_ = saveData.SetPoint();
    rctx_.renderer_.DrawSetPoint(
        saveData.SetPoint().GetUnitWhole(saveData.TemperatureUnit()));
  }
}

void CoolableParent::ActivateCoolingRelays(Relay onRelay,
                                           Relay offRelay,
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
    ActivateCoolingRelays(Relay::Compressor, Relay::Heat,
                          ReverseValveModeT::OnForCooling);
    machine_.WriteHaActionStateTopicResponse(HaActionKey::Cooling);
  } else {
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
