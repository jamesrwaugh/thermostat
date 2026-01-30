#pragma once

#include <stdint.h>

class Noritake_VFD_GU7000;

constexpr uint8_t ImageWidth = 5;
constexpr uint8_t ImageHeight = 7;

extern const uint8_t gBlankImageData[ImageWidth];
extern const uint8_t gUpArrowImageData[ImageWidth];
extern const uint8_t gDownArrowImageData[ImageWidth];
extern const uint8_t gFireOneImageData[ImageWidth];
extern const uint8_t gFireTwoImageData[ImageWidth];
extern const uint8_t gSnowflakeOneImageData[ImageWidth];
extern const uint8_t gSnowflakeTwoImageData[ImageWidth];

void DrawImage(Noritake_VFD_GU7000& screen, uint8_t xPositionDots, bool bottom,
               const uint8_t image[ImageWidth]);
