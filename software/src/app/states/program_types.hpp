#pragma once

#include <etl/alignment.h>
#include <stdint.h>

class Noritake_VFD_GU7000;
struct ThermoSaveData;

constexpr uint8_t CharDotWidth = 7;
constexpr uint8_t CharDotHeight = 7;

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

 private:
  const uint8_t xPosChars;
  const uint8_t Min;
  const uint8_t Max;
};

class TwoDigitScreenBox : public ScreenBox {
 public:
  using ScreenBox::ScreenBox;
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

typedef etl::aligned_storage<sizeof(ScreenBox), alignof(ScreenBox)>::type
    ScreenBoxStorage;

// ================================================================ //

struct StaticScreenBox {
  StaticScreenBox(uint8_t xPosChars, char Character);
  const uint8_t xPosChars;
  const char Character;
};

typedef etl::aligned_storage<sizeof(StaticScreenBox),
                             alignof(StaticScreenBox)>::type
    StaticScreenBoxStorage;

// ================================================================ //

constexpr uint8_t ImageWidth = 7;
extern const uint8_t gArrowImageData[ImageWidth];
extern const uint8_t gBlankImageData[ImageWidth];
