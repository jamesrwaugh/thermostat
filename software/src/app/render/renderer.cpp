#include "renderer.hpp"

#include <Noritake_VFD_GU7000.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

#include <driver_rs_wrapper.hpp>

#include "digit_ops.hpp"

inline constexpr Image2xId I2xId(uint8_t number) {
  return static_cast<Image2xId>(number);
}

Renderer::Renderer(Noritake_VFD_GU7000& s, DigitImages& images)
    : screen_(s), images_{images} {}

void Renderer::InitializeDigitImages(Temperature::WholeType temperature,
                                     Humidity::WholeType humidity) {
  auto hundreds_image = Image2xId::Blank;

  if (temperature < 0) {
    hundreds_image = Image2xId::Minus;
  } else if (temperature >= 100) {
    hundreds_image = I2xId(Hundreds(temperature));
  } else {
    hundreds_image = Image2xId::Blank;
  }

  LoadImage2x(&images_.temperature_hundreds_or_minus_, hundreds_image);
  LoadImage2x(&images_.temperature_tens_, I2xId(Tens(temperature)));
  LoadImage2x(&images_.temperature_ones_, I2xId(Ones(temperature)));
  LoadImage2x(&images_.humidity_tens_, I2xId(Tens(humidity)));
  LoadImage2x(&images_.humidity_ones_, I2xId(Ones(humidity)));
}

void Renderer::DrawTemperature(TemperatureUnitT unit) {
  AutoTwi t;

  Draw2xImage(TemperatureXPos, images_.temperature_hundreds_or_minus_);

  DrawPositiveTwoDigitNumber(
      TemperatureXPos + Image2xWidth, images_.temperature_tens_,
      images_.temperature_ones_, unit == TemperatureUnitT::Celsius ? 'C' : 'F');
}

void Renderer::DrawHumidity() {
  AutoTwi t;
  DrawPositiveTwoDigitNumber(HumidityXPos, images_.humidity_tens_,
                             images_.humidity_ones_, '%');
}

void Renderer::DrawSetPoint(Temperature::WholeType setPoint) {
  AutoTwi t;

  uint8_t setPointPos = ScreenWidth;

  if (setPoint < 0) {
    setPointPos -= CharacterWidth1x;
  }

  const Temperature::WholeType posSetPoint = abs(setPoint);

  if (posSetPoint >= 100) {
    setPointPos -= 3 * CharacterWidth1x;
  } else if (posSetPoint >= 10) {
    setPointPos -= 2 * CharacterWidth1x;
  } else {
    setPointPos -= 1 * CharacterWidth1x;
  }

  if (setPointPos > last_setpoint_pos_) {
    constexpr uint8_t minSetPointPos = ScreenWidth - (4 * CharacterWidth1x);
    screen_.print(minSetPointPos, 0, "    ");
  }

  screen_.print(setPointPos, 0, setPoint, 10);

  last_setpoint_pos_ = setPointPos;
}

void Renderer::DrawHeatingStatus(HeatModeT heatMode, bool active) {
  AutoTwi t;

  const uint8_t imagePosition = ScreenWidth - Image1xWidth - 1;
  const uint8_t onPositionX = imagePosition - (CharacterWidth1x * 2) - 2;
  const uint8_t onPositionY = 9;

  Image1xId img;

  if (active) {
    const Image1xId startImg =
        heatMode == HeatModeT::Heating ? Image1xId::Fire0 : Image1xId::Cold0;
    status_image_idx_ = (status_image_idx_ + 1) % 9;
    img = static_cast<Image1xId>(startImg + status_image_idx_);
  } else {
    if (heatMode == HeatModeT::Heating) {
      img = Image1xId::Fire3;
    } else if (heatMode == HeatModeT::Cooling) {
      img = Image1xId::Cold0;
    } else {
      img = Image1xId::Idle0;
    }
  }

  screen_.print(onPositionX, onPositionY, active ? "ON" : "  ");
  Draw1xImage(imagePosition, true, img);
}

void Renderer::DrawPositiveTwoDigitNumber(uint8_t xPos,
                                          const Image2x& tens,
                                          const Image2x& ones,
                                          char suffix) {
  const uint8_t tensSpot = xPos;
  const uint8_t onesSpot = tensSpot + Image2xWidth + 1;
  const uint8_t suffixSpot = onesSpot + Image2xWidth + 1;

  Draw2xImage(tensSpot, tens);
  Draw2xImage(onesSpot, ones);

  screen_.print(suffixSpot, 1, suffix);
}

void Renderer::Draw2xImage(uint8_t x, const Image2x& image) {
  screen_.GU7000_drawImage(x, BigDigitYPos, Image2xWidth, Image2xHeight, image);
}

void Renderer::Draw1xImage(uint8_t xPositionDots,
                           bool bottom,
                           Image1xId imageId) {
  Image1x image;
  LoadImage1x(&image, imageId);
  screen_.GU7000_drawImage(xPositionDots, Image1xHeight + 1, Image1xWidth,
                           bottom ? 8 : 0, image);
}
