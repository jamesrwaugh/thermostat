#pragma once

#include "event.hpp"
#include "state.hpp"

class Machine;

class CoolableParent : public State::Base {
 public:
  CoolableParent(Machine& machine, State::Type stateId);
  virtual ~CoolableParent();
  State::Type handle_event(const Event::Base& event) override;

 protected:
  void ActivateCoolingRelays(Relay onRelay, Relay offRelay,
                             ReverseValveModeT onIfType);
  void EnterHeatingOrCooling(HeatModeT mode);
  void ExitHeatingOrCooling();
  Machine& machine_;

 private:
  void TickChangeCounter();
  [[nodiscard]] bool HasChangeTimeoutPassed() const;
  [[nodiscard]] State::Type ChangeSetPoint(int8_t change);
  [[nodiscard]] State::Type DetermineNextState();
  [[nodiscard]] bool IsHeatingOrCooling() const;
  [[nodiscard]] bool IsIdle() const;

  struct StateChangeData {
    static constexpr uint8_t MaxStateChangeTimeoutSec = 10;
    uint8_t StateChangeTimeoutSec{0};
  };

  StateChangeData ChData;
};
