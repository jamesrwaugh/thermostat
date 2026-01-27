#include "program_types.hpp"

#include <Noritake_VFD_GU7000.h>
#include <ThermoSaveData_bp.h>
#include <etl/algorithm.h>
#include <etl/placement_new.h>
#include <stdint.h>

#include <driver_rs_wrapper.hpp>

#include "images.hpp"

// ================================================================ //

Noritake_VFD_GU7000* ScreenBox::Screen_ = nullptr;

ScreenBox::ScreenBox(uint8_t xPosChars, uint8_t* targetData, uint8_t min,
                     uint8_t max)
    : TargetData_{targetData}, xPosChars{xPosChars}, Min{min}, Max{max} {}

void ScreenBox::Up() {
  uint8_t* data = TargetData_;
  if (*data < Max) {
    *data += 1;
  } else {
    *data = Min;
  }
  Draw();
}

void ScreenBox::Down() {
  uint8_t* data = TargetData_;
  if (*data > Min) {
    *data -= 1;
  } else {
    *data = Max;
  }
  Draw();
}

void ScreenBox::DrawIndicator() const {
  DrawIndicator(true);
}

void ScreenBox::DrawIndicator(bool on) const {
  AutoTwi t;
  Screen_->GU7000_drawImage(xPositionDots() + 1, CharDotHeight, ImageWidth, 8,
                            on ? gUpArrowImageData : gBlankImageData);
}

void ScreenBox::Draw() const {
  Draw(true);
}

uint8_t ScreenBox::xPositionDots() const {
  return xPosChars * CharDotWidth;
}

// ================================================================ //

void TwoDigitScreenBox::Draw(bool on) const {
  AutoTwi t;
  const auto pos = xPositionDots();
  if (on) {
    const uint8_t tens = *TargetData_ / 10;
    const uint8_t ones = *TargetData_ % 10;
    Screen_->print(pos, 0, static_cast<char>('0' + tens));
    Screen_->print(pos + CharDotWidth, 0, static_cast<char>('0' + ones));
  } else {
    Screen_->print(pos, 0, "  ", 2);
  }
}

// ================================================================ //

constexpr const char* FullScreenSpaceBuffer = "               ";
constexpr int FullScreenSpaceBufferLen = 15;

CharSetScreenBox::CharSetScreenBox(uint8_t xPosChars, uint8_t* targetData,
                                   const char* charSet, uint8_t charSetLength,
                                   uint8_t stride)
    : ScreenBox(
          xPosChars, targetData, 0,
          etl::min(FullScreenSpaceBufferLen, (charSetLength / stride) - 1)),
      CharSet{charSet},
      CharSetLength{charSetLength},
      Stride{stride} {}

void CharSetScreenBox::Draw(bool on) const {
  AutoTwi t;
  const auto pos = xPositionDots();
  if (on) {
    const char* charPtr = &CharSet[*TargetData_ * Stride];
    Screen_->print(pos, 0, charPtr, Stride);
  } else {
    Screen_->print(pos, 0, FullScreenSpaceBuffer, Stride);
  }
}

// ================================================================ //

StaticScreenBox::StaticScreenBox(uint8_t xPosChars, char character)
    : xPosChars{xPosChars}, Character{character} {}
