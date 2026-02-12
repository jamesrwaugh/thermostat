#include "scoller.hpp"

#include <Noritake_VFD_GU7000.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <driver_rs_wrapper.hpp>

// ================================================================= //

static const uint8_t image_0_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x3f, 0xf0, 0xc0, 0xcc, 0xc0, 0xcc,
  0xc3, 0x0c, 0xc3, 0x0c, 0xcc, 0x0c, 0xcc, 0x0c, 0x3f, 0xf0, 0x3f, 0xf0,
};

static const uint8_t image_1_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x0c, 0x30, 0x0c,
  0xff, 0xfc, 0xff, 0xfc, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t image_2_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x30, 0x0c, 0x30, 0x0c, 0xc0, 0x3c, 0xc0, 0x3c,
  0xc0, 0xcc, 0xc0, 0xcc, 0xc3, 0x0c, 0xc3, 0x0c, 0x3c, 0x0c, 0x3c, 0x0c};

static const uint8_t image_3_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0xc0, 0x30, 0xc0, 0x30, 0xc0, 0x0c, 0xc0, 0x0c,
  0xcc, 0x0c, 0xcc, 0x0c, 0xf3, 0x0c, 0xf3, 0x0c, 0xc0, 0xf0, 0xc0, 0xf0,
};

static const uint8_t image_4_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x03, 0xc0, 0x0c, 0xc0, 0x0c, 0xc0,
  0x30, 0xc0, 0x30, 0xc0, 0xff, 0xfc, 0xff, 0xfc, 0x00, 0xc0, 0x00, 0xc0,
};

static const uint8_t image_5_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0xfc, 0x30, 0xfc, 0x30, 0xcc, 0x0c, 0xcc, 0x0c,
  0xcc, 0x0c, 0xcc, 0x0c, 0xcc, 0x0c, 0xcc, 0x0c, 0xc3, 0xf0, 0xc3, 0xf0,
};

static const uint8_t image_6_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x0f, 0xf0, 0x33, 0x0c, 0x33, 0x0c,
  0xc3, 0x0c, 0xc3, 0x0c, 0xc3, 0x0c, 0xc3, 0x0c, 0x00, 0xf0, 0x00, 0xf0,
};

static const uint8_t image_7_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xc0, 0x00, 0xc0, 0xfc, 0xc0, 0xfc,
  0xc3, 0x00, 0xc3, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xf0, 0x00, 0xf0, 0x00,
};

static const uint8_t image_8_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x3c, 0xf0, 0x3c, 0xf0, 0xc3, 0x0c, 0xc3, 0x0c,
  0xc3, 0x0c, 0xc3, 0x0c, 0xc3, 0x0c, 0xc3, 0x0c, 0x3c, 0xf0, 0x3c, 0xf0,
};

static const uint8_t image_9_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x3c, 0x00, 0xc3, 0x0c, 0xc3, 0x0c,
  0xc3, 0x0c, 0xc3, 0x0c, 0xc3, 0x30, 0xc3, 0x30, 0x3f, 0xc0, 0x3f, 0xc0,
};

static const Image2x* const number_2x_images[10] = {
  &image_0_2x, &image_1_2x, &image_2_2x, &image_3_2x, &image_4_2x,
  &image_5_2x, &image_6_2x, &image_7_2x, &image_8_2x, &image_9_2x,
};

// ================================================================= //

Scroller::Scroller(uint8_t xPositionDots, uint8_t startingNumber)
    : x_position_dots_{xPositionDots}, current_number_{startingNumber} {
  SetNumber(current_number_);
}

void Scroller::ScrollInDirection(bool positive) {
  positive ? ScrollDownOneLine() : ScrollDownOneLine();
}

void Scroller::ScollUpOneLine() {
  for (uint8_t col = 0; col < ImageWidth2xFullSize; col += 2) {
    ScollUpColumn(col);
  }

  scrolled_lines_ += 1;

  if (scrolled_lines_ == ImageHeight2x) {
    scrolled_lines_ = 0;
    current_number_ = PrevNumber();
  }
}

void Scroller::ScrollDownOneLine() {
  for (uint8_t col = 0; col < ImageWidth2xFullSize; col += 2) {
    ScrollDownColumn(col);
  }

  scrolled_lines_ -= 1;

  if (scrolled_lines_ == -static_cast<int8_t>(ImageHeight2x)) {
    scrolled_lines_ = 0;
    current_number_ = NextNumber();
  }
}

int8_t Scroller::ScrolledCount() const {
  return scrolled_lines_;
}

uint8_t Scroller::CurrentNumber() const {
  return current_number_;
}

void Scroller::SetNumber(uint8_t n) {
  current_number_ = n;
  scrolled_lines_ = 0;
  memcpy(&image_, number_2x_images[current_number_], sizeof(Image2x));
}

void Scroller::Draw() {
  AutoTwi t;
  DriverGetScreenHandle().GU7000_drawImage(x_position_dots_, 0, ImageWidth2x,
                                           ImageHeight2x, image_);
}

void Scroller::ScrollDownColumn(uint8_t col) {
  uint8_t& top = image_[col];
  uint8_t& bottom = image_[col + 1];

  uint8_t nextNumber = HaveScrolledUp() ? current_number_ : NextNumber();

  const auto& nextImage = *number_2x_images[nextNumber];

  uint8_t row =
    HaveScrolledUp() ? (ImageHeight2x - scrolled_lines_) : abs(scrolled_lines_);

  uint8_t nextCol = nextImage[col + (row < 8 ? 1 : 0)];

  bottom >>= 1;
  bottom |= (top & 1) ? (1 << 7) : 0;
  top >>= 1;
  top |= (nextCol & (1 << (row % 8))) ? (1 << 7) : 0;
}

void Scroller::ScollUpColumn(uint8_t col) {
  uint8_t& top = image_[col];
  uint8_t& bottom = image_[col + 1];

  uint8_t prevNumber = HaveScrolledDown() ? current_number_ : PrevNumber();

  const auto& prevImage = *number_2x_images[prevNumber];

  uint8_t row =
    HaveScrolledDown() ? ImageHeight2x - abs(scrolled_lines_) : scrolled_lines_;

  uint8_t prevCol = prevImage[col + (row < 8 ? 0 : 1)];

  top <<= 1;
  top |= (bottom & (1 << 7)) ? 1 : 0;
  bottom <<= 1;
  bottom |= (prevCol & (1 << (7 - (row % 8)))) ? 1 : 0;
}

uint8_t Scroller::NextNumber() const {
  return current_number_ < 9 ? current_number_ + 1 : 0;
}

uint8_t Scroller::PrevNumber() const {
  return current_number_ == 0 ? 9 : current_number_ - 1;
}

bool Scroller::HaveScrolledUp() const {
  return scrolled_lines_ > 0;
}

bool Scroller::HaveScrolledDown() const {
  return scrolled_lines_ < 0;
}

// ================================================================= //