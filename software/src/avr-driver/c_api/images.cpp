#include "images.hpp"

#include <Noritake_VFD_GU7000.h>

#include "driver_rs_wrapper.hpp"

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

const uint8_t gFireOneImageData[ImageWidth] = {
    // clang-format off
    0b00011110,
    0b11111111,
    0b11111111,
    0b01111111,
    0b00000110,
    // clang-format on
};

const uint8_t gFireTwoImageData[ImageWidth] = {
    // clang-format off
    0b00000110,
    0b01111111,
    0b11111111,
    0b11111111,
    0b00001110,
    // clang-format on
};

const uint8_t gSnowflakeOneImageData[ImageWidth]{
    // clang-format off
    0b01000100,
    0b00101000,
    0b01010100,
    0b00101000,
    0b01000100,
    // clang-format on
};

const uint8_t gSnowflakeTwoImageData[ImageWidth]{
    // clang-format off
     0b01010100,
     0b00101000,
     0b00010000,
     0b00101000,
     0b01010100,
    // clang-format on
};

void DrawImage(uint8_t xPositionDots, bool bottom, Image image) {
  AutoTwi t;
  DriverGetScreenHandle().GU7000_drawImage(xPositionDots, ImageHeight,
                                           ImageWidth, bottom ? 8 : 0, image);
}
