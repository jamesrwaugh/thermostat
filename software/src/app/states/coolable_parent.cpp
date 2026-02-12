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

CoolableParent::CoolableParent(Machine& machine,
                               State::Type stateId,
                               const Image* const a,
                               const Image* const b)
    : State::Base(stateId),
      machine_(machine),
      status_image_a_{a},
      status_image_b_{b},
      t10s_(0, 0),
      t1s_(13, 0),
      h10s_(39, 0),
      h1s_(52, 0) {
  DriverDisplayClearScreen();
  machine_.DisplaySetPoint();

  const auto& t = machine_.CurrentTemperature().GetUnitWhole(
    machine.SaveState().TemperatureUnit());
  t10s_.SetNumber(t / 10);
  t1s_.SetNumber(t % 10);

  const auto& h = machine_.CurrentHumidity().ToWholePercent();
  h10s_.SetNumber(h / 10);
  h1s_.SetNumber(h % 10);

  t10s_.Draw();
  t1s_.Draw();
  h10s_.Draw();
  h1s_.Draw();

  {
    AutoTwi t;
    DriverGetScreenHandle().GU7000_setScreenBrightness(1);
  }
}

CoolableParent::~CoolableParent() {}

void ApplyScrollAndRedraw(Scroller& s, int8_t& remainingAmount) {
  if (remainingAmount != 0) {
    s.ScrollInDirection(remainingAmount > 0);
    s.Draw();
    if (remainingAmount > 0) {
      remainingAmount -= 1;
    } else {
      remainingAmount += 1;
    }
  }
}

class TwoNumberScroller {
  void TeleportSetNumber(int8_t number) {
    tens_.SetNumber(number / 10);
    ones_.SetNumber(number % 10);
  }

  void BeginScrollToNumber(int8_t number) {}

  void Draw() {
    AutoTwi t;
  }

  Scroller tens_;
  Scroller ones_;
};

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
      auto& s = scroll_state_;
      s.tick_ten_ms_count += 1;
      if (s.tick_ten_ms_count == 10) {
        s.tick_ten_ms_count = 0;
        ApplyScrollAndRedraw(t10s_, scroll_state_.t10s_lines_);
        ApplyScrollAndRedraw(t1s_, scroll_state_.t1s_lines_);
        ApplyScrollAndRedraw(h10s_, scroll_state_.h10s_lines_);
        ApplyScrollAndRedraw(h1s_, scroll_state_.h1s_lines_);
      }
      return State::Type::NO_CHANGE;
    }
    case Event::Type::SecondPassed: {
      TemperatureChangeInfo info;
      machine_.ReadTemperatureAndReportIfChanged(info);
      if (info.TemperatureChanged()) {
        const auto& n = info.NewTemperature.GetCelciusWhole();
        const auto& o = info.OldTemperature.GetCelciusWhole();
        scroll_state_.t10s_lines_ += ((n / 10) - (o / 10)) * ImageHeight2x;
        scroll_state_.t1s_lines_ += ((n & 10) - (o & 10)) * ImageHeight2x;
      }
      if (info.HumidityChanged()) {
        const auto& n = info.NewHumidity.ToWholePercent();
        const auto& o = info.OldHumidity.ToWholePercent();
        scroll_state_.h10s_lines_ += ((n / 10) - (o / 10)) * ImageHeight2x;
        scroll_state_.h1s_lines_ += ((n & 10) - (o & 10)) * ImageHeight2x;
      }
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
