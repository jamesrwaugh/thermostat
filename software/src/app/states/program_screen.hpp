#pragma once

#include <etl/alignment.h>
#include <stdint.h>

#include "state.hpp"

// ================================================================ //

class Noritake_VFD_GU7000;
struct ThermoSaveData;

class ScreenBox {
 public:
  static Noritake_VFD_GU7000* Screen_;

  ScreenBox(uint8_t xPosChars, uint8_t* targetData, uint8_t min, uint8_t max);

  void Up();
  void Down();
  void Draw() const;
  virtual void Draw(bool on) const = 0;
  void DrawIndicator() const;
  void DrawIndicator(bool on) const;
  uint8_t GetCurrentIndex() const;

 protected:
  uint8_t* const TargetData_;
  uint8_t xPositionDots() const;
  static constexpr uint8_t CharDotWidth = 7;
  static constexpr uint8_t CharDotHeight = 7;

 private:
  const uint8_t xPosChars;
  const uint8_t Min;
  const uint8_t Max;
};

class TwoDigitScreenBox : public ScreenBox {
 public:
  TwoDigitScreenBox(uint8_t xPosChars, uint8_t* targetData, uint8_t min,
                    uint8_t max);
  void Draw(bool on) const override;
};

class CharSetScreenBox : public ScreenBox {
 public:
  CharSetScreenBox(uint8_t xPosChars, uint8_t* targetData, const char* charSet,
                   uint8_t charSetLength, uint8_t stride);
  void Draw(bool on) const override;

 private:
  const char* const CharSet;
  const uint8_t CharSetLength;
  const uint8_t Stride;
};

// ================================================================ //

struct StaticScreenBox {
  StaticScreenBox(uint8_t xPosChars, char Character);
  const uint8_t xPosChars;
  const char Character;
};

typedef etl::aligned_storage<sizeof(ScreenBox), alignof(ScreenBox)>::type
    ScreenBoxStorage;

// ================================================================ //

typedef etl::aligned_storage<sizeof(StaticScreenBox),
                             alignof(StaticScreenBox)>::type
    StaticScreenBoxStorage;

class ProgramScreenState : public State::Base {
 public:
  ProgramScreenState(State::Type stateId, const char* title, ThermoSaveData& s,
                     uint8_t boxesCount, State::Type prevState,
                     State::Type nextState);
  ~ProgramScreenState();

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