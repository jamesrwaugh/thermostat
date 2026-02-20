#pragma once

#include <etl/alignment.h>
#include <stdint.h>

#include "ThermoSaveData_bp.h"
#include "driver_ds1307.h"
#include "program_types.hpp"
#include "state.hpp"

struct ScreenConfig {
  State::Type stateId;
  const char* title;
  uint8_t boxesCount;
  State::Type prevState;
  State::Type nextState;
};

class ProgramScreenState : public State::Base {
 public:
  ProgramScreenState(const ScreenConfig& s);

  void InitDisplay(bool startOnEndBox);
  State::Type handle_event(const Event::Base& event) override;
  [[nodiscard]] State::Type OnUpPressed();
  [[nodiscard]] State::Type OnDownPressed();
  void OnSelectPressed();
  void OnHalfSecondPassed();

 public:
  static Noritake_VFD_GU7000* Screen_;

 protected:
  ScreenBoxStorage Boxes_[5];
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
  bool ShowIndicator_{false};
  uint8_t CursorPosition{0};
  bool Locked_{false};
  bool HasEditedCurrentBox_{false};
  const ScreenConfig Config_;
};

// ================================================================ //

class TempScreen final : public ProgramScreenState {
 public:
  TempScreen(ThermoSaveData& s, bool startOnEndBox);

 private:
  ThermoSaveData& S_;
};

// ================================================================ //

class DateScreen final : public ProgramScreenState {
 public:
  DateScreen(ds1307_time_s& s, bool startOnEndBox);
  ~DateScreen();

 private:
  uint8_t year_{0};
  ds1307_time_s& time_;
};

// ================================================================ //

class TimeScreen final : public ProgramScreenState {
 public:
  TimeScreen(ds1307_time_s& s, bool startOnEndBox);
  ~TimeScreen();

 private:
  ds1307_time_s& time_;
  uint8_t am_pm_{0};
};
