#pragma once

#include <stdint.h>

constexpr uint8_t ImageWidth2x = 12;
constexpr uint8_t ImageWidth2xHalfSize = ImageWidth2x;
constexpr uint8_t ImageWidth2xFullSize = ImageWidth2x * 2;
constexpr uint8_t ImageHeight2x = 16;

typedef uint8_t Image2xHalf[ImageWidth2xHalfSize];
typedef uint8_t Image2x[ImageWidth2xFullSize];

typedef const Image2x* const Image2x0Thru9[10];

class Scroller {
 public:
  static_assert(ImageWidth2xFullSize % 2 == 0,
                "Image width must be multiple of 2");

  Scroller(uint8_t xPositionDots, uint8_t startingNumber);

  void ScrollInDirection(bool positive);
  void ScollUpOneLine();
  void ScrollDownOneLine();
  int8_t ScrolledCount() const;
  uint8_t CurrentNumber() const;
  void SetNumber(uint8_t n);
  void Draw();

 private:
  void ScrollDownColumn(uint8_t col);
  void ScollUpColumn(uint8_t col);
  uint8_t NextNumber() const;
  uint8_t PrevNumber() const;
  bool HaveScrolledUp() const;
  bool HaveScrolledDown() const;

  Image2x image_;
  const uint8_t x_position_dots_{0};
  uint8_t current_number_{0};
  int8_t scrolled_lines_{0};  // >0 scrolled up, <0 scrolled down
};
