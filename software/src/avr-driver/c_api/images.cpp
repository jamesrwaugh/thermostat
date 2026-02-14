#include "images.hpp"

const uint8_t gBlankImageData[ImageWidth] = {
  // clang-format off
    0b0000'0000,
    0b0000'0000,
    0b0000'0000,
    0b0000'0000,
    0b0000'0000,
  // clang-format on
};

const uint8_t gUpArrowImageData[ImageWidth] = {
  // clang-format off
    0b0000'1000,
    0b0001'1000,
    0b0011'1110,
    0b0001'1000,
    0b0000'1000,
  // clang-format on
};

const uint8_t gDownArrowImageData[ImageWidth] = {
  // clang-format off
    0b00010000,
    0b00011000,
    0b01111100,
    0b00011000,
    0b00010000,
  // clang-format on
};
