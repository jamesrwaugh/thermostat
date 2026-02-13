#include "renderer.hpp"

#include <stdlib.h>
#include <string.h>

#include "GU7000/Noritake_VFD_GU7000.h"
#include "autotwi.hpp"

// ==================================================== //

static const Image1x image_fire0 = {0x4e, 0x3f, 0x7f, 0x1f, 0x0e};
static const Image1x image_fire1 = {0x8e, 0x3f, 0x7f, 0x3f, 0x0e};
static const Image1x image_fire2 = {0x0e, 0x3f, 0x7f, 0xff, 0x06};
static const Image1x image_fire3 = {0x0e, 0xbf, 0xff, 0x7f, 0x26};
static const Image1x image_fire4 = {0x0e, 0xff, 0x3f, 0x1f, 0x4e};
static const Image1x image_fire5 = {0x3e, 0x7f, 0xbf, 0x1f, 0x8e};
static const Image1x image_fire6 = {0x3e, 0x1f, 0x7f, 0x1f, 0x06};
static const Image1x image_fire7 = {0x06, 0x1f, 0xbf, 0x1f, 0x0e};
static const Image1x image_fire8 = {0x2e, 0x1f, 0x7f, 0x3f, 0x0e};

static constexpr uint8_t FiresImagesCount = 9;

static const Image1x* fire_images[FiresImagesCount] = {
  &image_fire0, &image_fire1, &image_fire2, &image_fire3, &image_fire4,
  &image_fire5, &image_fire6, &image_fire7, &image_fire8,
};

static constexpr uint8_t DefaultHeatImageIdx = 3;

static_assert(DefaultHeatImageIdx < FiresImagesCount,
              "Default fire index out of bounds");

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

  screen_.GU7000_drawImage(TemperatureXPos, 0, ImageWidth2x, ImageHeight2x,
                           images_.temperature_hundreds_or_minus_);

  DrawPositive2DigitNumber(TemperatureXPos + ImageWidth2x,
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

void Renderer::DrawHeatingStatus(bool active) {
  AutoTwi t;

  const uint8_t imagePosition = ScreenWidth - Image1xWidth - 1;

  if (active) {
    screen_.print(imagePosition - (CharacterWidth1x * 2) - 2, 9, "ON");
    fire_state_ += 1;
    if (fire_state_ == FiresImagesCount) {
      fire_state_ = 0;
    }
    Draw1xImage(imagePosition, true, *fire_images[fire_state_]);
  } else {
    Draw1xImage(imagePosition, true, *fire_images[DefaultHeatImageIdx]);
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
  const uint8_t onesSpot = tensSpot + ImageWidth2x + 1;
  const uint8_t suffixSpot = onesSpot + ImageWidth2x + 1;

  screen_.GU7000_drawImage(tensSpot, 1, ImageWidth2x, ImageHeight2x, tens);

  screen_.GU7000_drawImage(onesSpot, 1, ImageWidth2x, ImageHeight2x, ones);

  screen_.print(suffixSpot, 1, suffix);
}

void Renderer::Draw1xImage(uint8_t xPositionDots,
                           bool bottom,
                           const Image1x& image) {
  screen_.GU7000_drawImage(xPositionDots, Image1xHeight + 1, Image1xWidth,
                           bottom ? 8 : 0, image);
}
