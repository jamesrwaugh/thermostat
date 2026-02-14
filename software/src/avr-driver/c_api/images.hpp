#pragma once

#include <stdint.h>

constexpr uint8_t ImageWidth = 5;
constexpr uint8_t ImageHeight = 7;

typedef const uint8_t Image[ImageWidth];

extern const uint8_t gBlankImageData[ImageWidth];
extern const uint8_t gUpArrowImageData[ImageWidth];
extern const uint8_t gDownArrowImageData[ImageWidth];
