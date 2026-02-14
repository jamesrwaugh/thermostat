#pragma once

#include <temperature_change.hpp>

#include "event.hpp"
#include "state.hpp"

class Machine;
class RenderContext;
class ScrollManager;

struct FrictionScrollManager {
 public:
  FrictionScrollManager(ScrollManager& s);
  bool AttemptScroll();

 private:
  uint8_t scroll_attempts_{0};
  uint8_t current_friction_{0};
  ScrollManager& s_;
};

class CoolableParent : public State::Base {
 public:
  CoolableParent(Machine& machine, State::Type stateId);
  virtual ~CoolableParent();
  State::Type handle_event(const Event::Base& event) override;

 protected:
  void Render();
  void ActivateCoolingRelays(Relay onRelay,
                             Relay offRelay,
                             ReverseValveModeT onIfType);
  void EnterHeatingOrCooling(HeatModeT mode);
  void ExitHeatingOrCooling();

  Machine& machine_;
  RenderContext& rctx_;
  Temperature last_set_point_;
  HeatModeT last_heating_mode{HeatModeT::None};
  uint8_t heating_render_count_{0};
  FrictionScrollManager temp_;
  FrictionScrollManager humid_;

 private:
  [[nodiscard]] State::Type ChangeSetPoint(bool increment);
  [[nodiscard]] State::Type DetermineNextState();
  [[nodiscard]] bool IsHeatingOrCooling() const;
  [[nodiscard]] bool IsIdle() const;
};
