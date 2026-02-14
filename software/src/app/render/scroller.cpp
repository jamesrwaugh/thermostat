#include "scroller.hpp"

#include <stdint.h>
#include <stdlib.h>

Scroller::Scroller(Image2x& image, Image2xId originalImage, Image2xId nextImage)
    : image_{image}, original_image_{originalImage}, next_image_{nextImage} {}

bool Scroller::ScrollInDirection(ScrollDirection dir) {
  return dir == ScrollDirection::Down ? ScrollDownOneLine() : ScollUpOneLine();
}

bool Scroller::ScollUpOneLine() {
  for (uint8_t col = 0; col < ImageWidth2xFullSize; col += 2) {
    ScollUpColumn(col);
  }

  scrolled_lines_ += 1;

  if (scrolled_lines_ == ImageHeight2x) {
    scrolled_lines_ = 0;
    return true;
  }

  return false;
}

bool Scroller::ScrollDownOneLine() {
  for (uint8_t col = 0; col < ImageWidth2xFullSize; col += 2) {
    ScrollDownColumn(col);
  }

  scrolled_lines_ -= 1;

  if (scrolled_lines_ == -static_cast<int8_t>(ImageHeight2x)) {
    scrolled_lines_ = 0;
    return true;
  }

  return false;
}

void Scroller::SetImages(Image2xId originalImage, Image2xId nextImage) {
  original_image_ = originalImage;
  next_image_ = nextImage;
}

int8_t Scroller::ScrolledCount() const {
  return scrolled_lines_;
}

void Scroller::ScrollDownColumn(uint8_t col) {
  uint8_t& top = image_[col];
  uint8_t& bottom = image_[col + 1];

  const auto& nextImageId = HaveScrolledUp() ? original_image_ : next_image_;

  uint8_t row = HaveScrolledUp() ? (ImageHeight2x - scrolled_lines_)
                                 : abs(scrolled_lines_);

  uint8_t nextCol = LoadImage2xByte(nextImageId, col + (row < 8 ? 1 : 0));

  bottom >>= 1;
  bottom |= (top & 1) ? (1 << 7) : 0;
  top >>= 1;
  top |= (nextCol & (1 << (row % 8))) ? (1 << 7) : 0;
}

void Scroller::ScollUpColumn(uint8_t col) {
  uint8_t& top = image_[col];
  uint8_t& bottom = image_[col + 1];

  const auto& prevImageId = HaveScrolledDown() ? original_image_ : next_image_;

  uint8_t row = HaveScrolledDown() ? ImageHeight2x - abs(scrolled_lines_)
                                   : scrolled_lines_;

  uint8_t prevCol = LoadImage2xByte(prevImageId, col + (row < 8 ? 0 : 1));

  top <<= 1;
  top |= (bottom & (1 << 7)) ? 1 : 0;
  bottom <<= 1;
  bottom |= (prevCol & (1 << (7 - (row % 8)))) ? 1 : 0;
}

bool Scroller::HaveScrolledUp() const {
  return scrolled_lines_ > 0;
}

bool Scroller::HaveScrolledDown() const {
  return scrolled_lines_ < 0;
}

// ================================================================= //
