#pragma once

#include <etl/alignment.h>
#include <stdint.h>

// ================================================================ //

class Noritake_VFD_GU7000;
struct ThermoSaveData;

class ScreenBox {
 public:
  static Noritake_VFD_GU7000* Screen_;

  ScreenBox(uint8_t groupOrder, uint8_t groupCount, const char* charSet,
            uint8_t charSetCount, uint8_t initialIndex);

  void Up();
  void Down();
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

typedef etl::aligned_storage<sizeof(ScreenBox), alignof(ScreenBox)>::type
    ScreenBoxStorage;

class ScreenC {
 public:
  ScreenC(const char* title, ThermoSaveData& s, ScreenBoxStorage* boxStorage,
          uint8_t boxesCount);
  ~ScreenC();

  void InitDisplay();
  void OnUpPressed();
  void OnDownPressed();
  void OnSelectPressed();
  void OnHalfSecondPassed();
  ScreenBox& CurrentBox() const;
  ScreenBox& GetBox(uint8_t i) const;
  inline ScreenBox* GetBoxP(uint8_t i) const;

 public:
  static Noritake_VFD_GU7000* Screen_;

 protected:
  ThermoSaveData& SaveData_;
  ScreenBoxStorage* const Boxes_;
  const uint8_t BoxesCount_;

 private:
  const char* const Title;
  bool ShowIndicator_{false};
  uint8_t CursorPosition{0};
  bool Locked_{false};
};

class TempScreen : public ScreenC {
 public:
  TempScreen(ThermoSaveData& s, ScreenBoxStorage* boxStorage);
  ~TempScreen();
};