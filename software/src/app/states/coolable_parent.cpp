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
      rctx_{machine.GetRenderContext()},
      temp_{rctx_.temperature_manager_},
      humid_{rctx_.humidity_manager_} {}

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
      Render();
      return State::Type::NO_CHANGE;
    }
    case Event::Type::SecondPassed: {
      machine_.ReadTemperatureAndReportIfChanged();
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

  const auto heatMode = saveData.HeatMode();
  const auto setPoint = saveData.SetPoint();
  const auto tempUnit = saveData.TemperatureUnit();

  if (temp_.AttemptScroll()) {
    rctx_.renderer_.DrawTemperature(tempUnit);
  }

  if (humid_.AttemptScroll()) {
    rctx_.renderer_.DrawHumidity();
  }

  if (IsHeatingOrCooling()) {
    if (heating_render_count_++ >= 10) {
      heating_render_count_ = 0;
      rctx_.renderer_.DrawHeatingStatus(heatMode, true);
    }
  } else {
    rctx_.renderer_.DrawHeatingStatus(heatMode, false);
  }

  if (last_set_point_ != setPoint) {
    last_set_point_ = setPoint;
    rctx_.renderer_.DrawSetPoint(setPoint.GetUnitWhole(tempUnit));
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

// ===================================================================== //

uint16_t max(uint16_t a, uint16_t b) {
  return a > b ? a : b;
}

FrictionScrollManager::FrictionScrollManager(ScrollManager& s) : s_{s} {}

bool FrictionScrollManager::AttemptScroll() {
  if (!s_.IsFinished()) {
    scroll_attempts_ += 8;
    if (scroll_attempts_ >= current_friction_) {
      scroll_attempts_ = 0;
      s_.ScrollOnce();
      bool addAggressive = s_.ScrollLinesLeft() < (Image2xHeight / 2);
      current_friction_ = max(20, current_friction_ + (addAggressive ? 12 : 1));
      return true;
    }
  } else {
    scroll_attempts_ = 0;
    current_friction_ = 0;
  }
  return false;
}
