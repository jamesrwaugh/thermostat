#pragma once

#include <temperature_change.hpp>

#include "event.hpp"
#include "state.hpp"

class Machine;
class RenderContext;

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
  uint8_t render_count_{0};

 private:
  [[nodiscard]] State::Type ChangeSetPoint(bool increment);
  [[nodiscard]] State::Type DetermineNextState();
  [[nodiscard]] bool IsHeatingOrCooling() const;
  [[nodiscard]] bool IsIdle() const;
};
