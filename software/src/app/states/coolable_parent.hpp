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
  [[nodiscard]] bool IsHeatingOrCooling() const;
  [[nodiscard]] bool IsIdle() const;
  Machine& machine_;
};
