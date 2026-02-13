#pragma once

#include <stdint.h>

constexpr uint8_t ImageWidth2x = 12;
constexpr uint8_t ImageWidth2xHalfSize = ImageWidth2x;
constexpr uint8_t ImageWidth2xFullSize = ImageWidth2x * 2;
constexpr uint8_t ImageHeight2x = 16;

typedef uint8_t Image2x[ImageWidth2xFullSize];

typedef const Image2x* const Image2x_0Thru9[10];

extern const Image2x minus_2x;
extern const Image2x blank_2x;
extern const Image2x image_0_2x;
extern const Image2x image_1_2x;
extern const Image2x image_2_2x;
extern const Image2x image_3_2x;
extern const Image2x image_4_2x;
extern const Image2x image_5_2x;
extern const Image2x image_6_2x;
extern const Image2x image_7_2x;
extern const Image2x image_8_2x;
extern const Image2x image_9_2x;

extern const Image2x_0Thru9 number_2x_images;

enum class ScrollDirection : uint8_t {
  Up = 0,
  Down,
};

class ScrollerT {
 public:
  static_assert(ImageWidth2xFullSize % 2 == 0,
                "Image width must be multiple of 2");

  ScrollerT(Image2x& image, const Image2x& nextImage);

  bool ScrollInDirection(ScrollDirection dir);
  bool ScollUpOneLine();
  bool ScrollDownOneLine();
  void SetNextImage(const Image2x& image);
  int8_t ScrolledCount() const;

 private:
  void ScrollDownColumn(uint8_t col);
  void ScollUpColumn(uint8_t col);
  bool HaveScrolledUp() const;
  bool HaveScrolledDown() const;

  Image2x& image_;
  Image2x original_image_;
  const Image2x* next_image_;
  int8_t scrolled_lines_{0};  // >0 scrolled up, <0 scrolled down
};
