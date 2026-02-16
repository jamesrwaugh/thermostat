#pragma once

#include <images.h>

#include <temperature.hpp>
#include <thermo_data_types.hpp>

class Noritake_VFD_GU7000;

struct DigitImages {
  Image2x temperature_hundreds_or_minus_;
  Image2x temperature_tens_;
  Image2x temperature_ones_;
  Image2x humidity_tens_;
  Image2x humidity_ones_;
};

// ==================================================== //

class Renderer {
 public:
  Renderer(Noritake_VFD_GU7000& s, DigitImages& images);

  void InitializeDigitImages(Temperature::WholeType temperature,
                             Humidity::WholeType humidity);
  void DrawTemperature(TemperatureUnitT unit);
  void DrawHumidity();
  void DrawSetPoint(Temperature::WholeType setPoint);
  void DrawHeatingStatus(HeatModeT heatMode, bool active);

 private:
  static constexpr uint8_t BigDigitYPos = 1;
  static constexpr uint8_t CharacterWidth1x = 7;
  static constexpr uint8_t ScreenWidth = 112;
  static constexpr uint8_t TemperatureXPos = 0;
  static constexpr uint8_t HumidityXPos = TemperatureXPos + (3 * Image2xWidth) +
                                          CharacterWidth1x + CharacterWidth1x;

  void DrawPositive2DigitNumber(uint8_t xPos,
                                const Image2x& tens,
                                const Image2x& ones,
                                char suffix);

  void Draw1xImage(uint8_t xPositionDots, bool bottom, Image1xId imageId);

  uint8_t status_image_idx_{0};
  uint8_t last_setpoint_pos_{0};
  Noritake_VFD_GU7000& screen_;
  DigitImages& images_;
};