#pragma once

#include "data_types.hpp"
#include "scroller.hpp"

class Noritake_VFD_GU7000;

constexpr uint8_t Image1xWidth = 5;
constexpr uint8_t Image1xHeight = 7;
typedef const uint8_t Image1x[Image1xWidth];

class Renderer {
 public:
  static constexpr uint8_t CharacterWidth1x = 7;
  static constexpr uint8_t ScreenWidth = 112;
  static constexpr uint8_t TemperatureXPos = 0;
  static constexpr uint8_t HumidityXPos = TemperatureXPos + (3 * Image2xWidth) +
                                          CharacterWidth1x + CharacterWidth1x;

  struct Images {
    Image2x temperature_hundreds_or_minus_;
    Image2x temperature_tens_;
    Image2x temperature_ones_;
    Image2x humidity_tens_;
    Image2x humidity_ones_;
  };

  Renderer(Noritake_VFD_GU7000& s);

  void InitializeImages(int8_t temperature, uint8_t humidity);
  void DrawTemperature(char suffix);
  void DrawHumidity();
  void DrawSetPoint(int8_t setPoint);
  void DrawHeatingStatus(HeatModeT heatMode, bool active);
  Images& GetImages();

 private:
  void DrawPositive2DigitNumber(uint8_t xPos,
                                const Image2x& tens,
                                const Image2x& ones,
                                char suffix);

  void Draw1xImage(uint8_t xPositionDots, bool bottom, const Image1x& image);

  uint8_t status_image_idx_{0};
  Images images_;
  Noritake_VFD_GU7000& screen_;
};