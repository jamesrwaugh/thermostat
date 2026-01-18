#pragma once

#include <etl/alignment.h>
#include <stdint.h>

#include "program_types.hpp"
#include "state.hpp"

class ProgramScreenState : public State::Base {
 public:
  ProgramScreenState(State::Type stateId, const char* title, ThermoSaveData& s,
                     uint8_t boxesCount, State::Type prevState,
                     State::Type nextState);

  void InitDisplay(bool startOnEndBox);
  State::Type handle_event(const Event::Base& event) override;
  [[nodiscard]] State::Type OnUpPressed();
  [[nodiscard]] State::Type OnDownPressed();
  void OnSelectPressed();
  void OnHalfSecondPassed();

 public:
  static Noritake_VFD_GU7000* Screen_;

 protected:
  ThermoSaveData& SaveData_;
  ScreenBoxStorage Boxes_[5];
  uint8_t BoxesCount_{0};
  StaticScreenBoxStorage Statics_[5];
  uint8_t StaticsCount_{0};

 protected:
  const ScreenBox& CurrentBox() const;
  ScreenBox& CurrentBox();
  const ScreenBox& GetBox(uint8_t i) const;
  ScreenBox& GetBox(uint8_t i);
  ScreenBox* GetBoxP(uint8_t i);
  uint8_t GetBoxIndex(uint8_t i) const;
  void AddStatic(uint8_t xPosChars, char Character);

 private:
  const char* const Title;
  bool ShowIndicator_{false};
  uint8_t CursorPosition{0};
  bool Locked_{false};
  bool HasEditedCurrentBox_{false};
  const State::Type PrevState_;
  const State::Type NextState_;
};

// ================================================================ //

class TempScreen final : public ProgramScreenState {
 public:
  TempScreen(ThermoSaveData& s, bool startOnEndBox);
};

// ================================================================ //

class DateScreen final : public ProgramScreenState {
 public:
  DateScreen(ThermoSaveData& s, bool startOnEndBox);
};

// ================================================================ //

class TimeScreen final : public ProgramScreenState {
 public:
  TimeScreen(ThermoSaveData& s, bool startOnEndBox);
};
