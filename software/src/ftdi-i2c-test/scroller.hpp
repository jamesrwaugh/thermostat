#pragma once

#include <stdint.h>

constexpr uint8_t ImageWidth2x = 12;
constexpr uint8_t ImageWidth2xHalfSize = ImageWidth2x;
constexpr uint8_t ImageWidth2xFullSize = ImageWidth2x * 2;
constexpr uint8_t ImageHeight2x = 16;

typedef uint8_t Image2xHalf[ImageWidth2xHalfSize];
typedef uint8_t Image2x[ImageWidth2xFullSize];

typedef const Image2x* const Image2x0Thru9[10];

extern const uint8_t minus_2x[ImageWidth2xFullSize];
extern const uint8_t blank_2x[ImageWidth2xFullSize];
extern const uint8_t image_0_2x[ImageWidth2xFullSize];
extern const uint8_t image_1_2x[ImageWidth2xFullSize];
extern const uint8_t image_2_2x[ImageWidth2xFullSize];
extern const uint8_t image_3_2x[ImageWidth2xFullSize];
extern const uint8_t image_4_2x[ImageWidth2xFullSize];
extern const uint8_t image_5_2x[ImageWidth2xFullSize];
extern const uint8_t image_6_2x[ImageWidth2xFullSize];
extern const uint8_t image_7_2x[ImageWidth2xFullSize];
extern const uint8_t image_8_2x[ImageWidth2xFullSize];
extern const uint8_t image_9_2x[ImageWidth2xFullSize];

extern const Image2x* const number_2x_images[10];

enum class ScrollDirection : uint8_t {
  Up = 0,
  Down,
};

class ScrollerT {
 public:
  static_assert(ImageWidth2xFullSize % 2 == 0,
                "Image width must be multiple of 2");

  ScrollerT(Image2x& image, const Image2x& nextImage);

  void ScrollInDirection(ScrollDirection dir);
  void ScollUpOneLine();
  void ScrollDownOneLine();
  int8_t ScrolledCount() const;

 private:
  void ScrollDownColumn(uint8_t col);
  void ScollUpColumn(uint8_t col);
  bool HaveScrolledUp() const;
  bool HaveScrolledDown() const;

  Image2x& image_;
  Image2x original_image_;
  const Image2x& next_image_;
  int8_t scrolled_lines_{0};  // >0 scrolled up, <0 scrolled down
};
