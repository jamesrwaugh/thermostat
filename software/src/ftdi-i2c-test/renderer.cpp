#include "renderer.hpp"

#include <stdlib.h>
#include <string.h>

#include "GU7000/Noritake_VFD_GU7000.h"
#include "autotwi.hpp"

// ==================================================== //

static constexpr uint8_t AnimationSetCount = 9;
typedef Image1x* Animation1xSet[AnimationSetCount];

static const Image1x image_fire0 = {0x4e, 0x3f, 0x7f, 0x1f, 0x0e};
static const Image1x image_fire1 = {0x8e, 0x3f, 0x7f, 0x3f, 0x0e};
static const Image1x image_fire2 = {0x0e, 0x3f, 0x7f, 0xff, 0x06};
static const Image1x image_fire3 = {0x0e, 0xbf, 0xff, 0x7f, 0x26};
static const Image1x image_fire4 = {0x0e, 0xff, 0x3f, 0x1f, 0x4e};
static const Image1x image_fire5 = {0x3e, 0x7f, 0xbf, 0x1f, 0x8e};
static const Image1x image_fire6 = {0x3e, 0x1f, 0x7f, 0x1f, 0x06};
static const Image1x image_fire7 = {0x06, 0x1f, 0xbf, 0x1f, 0x0e};
static const Image1x image_fire8 = {0x2e, 0x1f, 0x7f, 0x3f, 0x0e};

static const Animation1xSet fire_images = {
    &image_fire0, &image_fire1, &image_fire2, &image_fire3, &image_fire4,
    &image_fire5, &image_fire6, &image_fire7, &image_fire8,
};

static constexpr uint8_t DefaultHeatImageIdx = 3;

static_assert(DefaultHeatImageIdx < AnimationSetCount,
              "Default fire index out of bounds");

// ==================================================== //

static const Image1x image_cold0 = {0x22, 0x14, 0x6b, 0x14, 0x22};
static const Image1x image_cold1 = {0x14, 0x6b, 0x14, 0x22, 0x00};
static const Image1x image_cold2 = {0x0a, 0x35, 0x0a, 0x11, 0x00};
static const Image1x image_cold3 = {0x08, 0x05, 0x1a, 0x05, 0x08};
static const Image1x image_cold4 = {0x00, 0xc4, 0x02, 0x8d, 0x02};
static const Image1x image_cold5 = {0x40, 0x80, 0x62, 0x81, 0x46};
static const Image1x image_cold6 = {0x00, 0x20, 0x40, 0xb1, 0x40};
static const Image1x image_cold7 = {0x00, 0x88, 0x50, 0xac, 0x50};
static const Image1x image_cold8 = {0x44, 0x28, 0xd6, 0x28, 0x44};

static const Animation1xSet snowflake_images = {
    &image_cold0, &image_cold1, &image_cold2, &image_cold3, &image_cold4,
    &image_cold5, &image_cold6, &image_cold7, &image_cold8,
};

static constexpr uint8_t DefaultSnowImageIdx = 0;

static_assert(DefaultSnowImageIdx < AnimationSetCount,
              "Default snow index out of bounds");

// ==================================================== //

static const Image1x image_idle = {0x3a, 0x44, 0x5a, 0x22, 0x5c};

// ==================================================== //

Renderer::Renderer(Noritake_VFD_GU7000& s) : screen_(s) {}

void Renderer::InitializeImages(int8_t temperature, uint8_t humidity) {
  const uint8_t posTemp = abs(temperature);

  const Image2x* hundreds_image = &blank_2x;

  if (temperature < 0) {
    hundreds_image = &minus_2x;
  } else if (temperature >= 100) {
    hundreds_image = number_2x_images[posTemp / 100];
  } else {
    hundreds_image = &blank_2x;
  }

  memcpy(&images_.temperature_hundreds_or_minus_, hundreds_image,
         sizeof(Image2x));
  memcpy(&images_.temperature_tens_, number_2x_images[(posTemp % 100) / 10],
         sizeof(Image2x));
  memcpy(&images_.temperature_ones_, number_2x_images[posTemp % 10],
         sizeof(Image2x));
  memcpy(&images_.humidity_tens_, number_2x_images[humidity / 10],
         sizeof(Image2x));
  memcpy(&images_.humidity_ones_, number_2x_images[humidity % 10],
         sizeof(Image2x));
}

void Renderer::DrawTemperature(char suffix) {
  AutoTwi t;

  screen_.GU7000_drawImage(TemperatureXPos, 0, Image2xWidth, Image2xHeight,
                           images_.temperature_hundreds_or_minus_);

  DrawPositive2DigitNumber(TemperatureXPos + Image2xWidth,
                           images_.temperature_tens_, images_.temperature_ones_,
                           suffix);
}

void Renderer::DrawHumidity() {
  AutoTwi t;
  DrawPositive2DigitNumber(HumidityXPos, images_.humidity_tens_,
                           images_.humidity_ones_, '%');
}

void Renderer::DrawSetPoint(int8_t setPoint) {
  AutoTwi t;

  uint8_t setPointPos = ScreenWidth;

  if (setPoint < 0) {
    setPointPos -= CharacterWidth1x;
  }

  static const uint8_t posSetPoint = abs(setPoint);

  if (posSetPoint >= 100) {
    setPointPos -= 3 * CharacterWidth1x;
  } else if (posSetPoint >= 10) {
    setPointPos -= 2 * CharacterWidth1x;
  } else {
    setPointPos -= 1 * CharacterWidth1x;
  }

  screen_.print(setPointPos, 0, setPoint, 10);
}

void Renderer::DrawHeatingStatus(HeatModeT heatMode, bool active) {
  AutoTwi t;

  const uint8_t imagePosition = ScreenWidth - Image1xWidth - 1;

  if (heatMode == HeatModeT::None) {
    Draw1xImage(imagePosition, true, image_idle);
  } else {
    const uint8_t defaultIndex = heatMode == HeatModeT::Heating
                                     ? DefaultHeatImageIdx
                                     : DefaultSnowImageIdx;
    const auto& imageSet =
        heatMode == HeatModeT::Heating ? fire_images : snowflake_images;

    if (active) {
      screen_.print(imagePosition - (CharacterWidth1x * 2) - 2, 9, "ON");
      status_image_idx_ = (status_image_idx_ + 1) % AnimationSetCount;
      Draw1xImage(imagePosition, true, *imageSet[status_image_idx_]);
    } else {
      Draw1xImage(imagePosition, true, *imageSet[defaultIndex]);
    }
  }
}

Renderer::Images& Renderer::GetImages() {
  return images_;
}

void Renderer::DrawPositive2DigitNumber(uint8_t xPos,
                                        const Image2x& tens,
                                        const Image2x& ones,
                                        char suffix) {
  const uint8_t tensSpot = xPos;
  const uint8_t onesSpot = tensSpot + Image2xWidth + 1;
  const uint8_t suffixSpot = onesSpot + Image2xWidth + 1;

  screen_.GU7000_drawImage(tensSpot, 1, Image2xWidth, Image2xHeight, tens);

  screen_.GU7000_drawImage(onesSpot, 1, Image2xWidth, Image2xHeight, ones);

  screen_.print(suffixSpot, 1, suffix);
}

void Renderer::Draw1xImage(uint8_t xPositionDots,
                           bool bottom,
                           const Image1x& image) {
  screen_.GU7000_drawImage(xPositionDots, Image1xHeight + 1, Image1xWidth,
                           bottom ? 8 : 0, image);
}
