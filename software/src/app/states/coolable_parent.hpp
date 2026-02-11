#pragma once

#include "event.hpp"
#include "images.hpp"
#include "scoller.hpp"
#include "state.hpp"

class Machine;

class CoolableParent : public State::Base {
 public:
  CoolableParent(Machine& machine,
                 State::Type stateId,
                 const Image* const a,
                 const Image* const b);
  virtual ~CoolableParent();
  State::Type handle_event(const Event::Base& event) override;

 protected:
  void ActivateCoolingRelays(Relay onRelay,
                             Relay offRelay,
                             ReverseValveModeT onIfType);
  void EnterHeatingOrCooling(HeatModeT mode);
  void ExitHeatingOrCooling();
  Machine& machine_;

 private:
  [[nodiscard]] State::Type ChangeSetPoint(bool increment);
  [[nodiscard]] State::Type DetermineNextState();
  [[nodiscard]] bool IsHeatingOrCooling() const;
  [[nodiscard]] bool IsIdle() const;

  bool image_state_{false};
  const Image* const status_image_a_;
  const Image* const status_image_b_;

  uint8_t tick_ten_ms_count_{0};
  uint8_t move_temperature_ticks_{0};

  Scroller t10s_;
  Scroller t1s_;
  Scroller h10s_;
  Scroller h1s_;
};
