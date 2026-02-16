#pragma once

#include <images.h>
#include <stdint.h>

// ==================================================== //

enum class ScrollDirection : uint8_t {
  Up = 0,
  Down,
};

class Scroller {
 public:
  static_assert(
      Image2xWidthFullSize % 2 == 0,
      "Image width must be multiple of 2 in order to iterate columns");

  Scroller(Image2x& image, Image2xId originalImage, Image2xId nextImage);

  bool ScrollInDirection(ScrollDirection dir);
  bool ScollUpOneLine();
  bool ScrollDownOneLine();
  void SetImages(Image2xId originalImage, Image2xId nextImage);
  int8_t ScrolledCount() const;

 private:
  void ScrollDownColumn(uint8_t col);
  void ScollUpColumn(uint8_t col);
  bool HaveScrolledUp() const;
  bool HaveScrolledDown() const;

  Image2x& image_;
  Image2xId original_image_;
  Image2xId next_image_;
  int8_t scrolled_lines_{0};  // >0 scrolled up, <0 scrolled down
};

// ==================================================== //