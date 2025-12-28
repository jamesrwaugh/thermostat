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

  ScreenBox(uint8_t xPosChars, const char* charSet, uint8_t charSetCount,
            uint8_t initialIndex);

  void Up();
  void Down();
  void Draw(bool on) const;
  void Draw() const;
  void DrawIndicator() const;
  void DrawIndicator(bool on) const;
  uint8_t GetCurrentIndex() const;

 private:
  uint8_t xPositionDots() const;
  static constexpr uint8_t CharDotWidth = 7;
  static constexpr uint8_t CharDotHeight = 7;
  const char* const CharSet;
  const uint8_t CharSetLength;
  const uint8_t xPosChars;
  uint8_t CharIndex{0};
};

// ================================================================ //

struct StaticScreenBox {
  StaticScreenBox(uint8_t xPosChars, char Character);
  const uint8_t xPosChars;
  const char Character;
};

// ================================================================ //

typedef etl::aligned_storage<sizeof(ScreenBox), alignof(ScreenBox)>::type
    ScreenBoxStorage;

typedef etl::aligned_storage<sizeof(StaticScreenBox),
                             alignof(StaticScreenBox)>::type
    StaticScreenBoxStorage;

class ProgramScreenState : public State::Base {
 public:
  ProgramScreenState(const char* title, ThermoSaveData& s, uint8_t boxesCount,
                     State::Type prevState, State::Type nextState);
  ~ProgramScreenState();

  void InitDisplay();
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
  ScreenBoxStorage Boxes_[8];
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

class TempScreen : public ProgramScreenState {
 public:
  TempScreen(ThermoSaveData& s);
  ~TempScreen();
};

// ================================================================ //

class DateScreen : public ProgramScreenState {
 public:
  DateScreen(ThermoSaveData& s);
  ~DateScreen();
};

// ================================================================ //

class TimeScreen : public ProgramScreenState {
 public:
  TimeScreen(ThermoSaveData& s);
  ~TimeScreen();
};