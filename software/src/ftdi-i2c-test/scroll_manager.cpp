#include "scroll_manager.hpp"

#include <stdlib.h>

ScrollManager::ScrollManager(Image2x& hundreds, Image2x& tens, Image2x& ones)
    : hundreds_scroller_(hundreds, blank_2x),
      tens_scroller_(tens, blank_2x),
      ones_scroller_(ones, blank_2x) {}

void ScrollManager::Calculate(int8_t old, int8_t theNew) {
  diff_direction_ =
      (theNew < old) ? ScrollDirection::Up : ScrollDirection::Down;
  current_number_ = old;
  goal_number_ = theNew;
  RecalculateImages();
}

bool ScrollManager::Finished() const {
  return current_number_ == goal_number_;
}

void ScrollManager::ApplyOnce() {
  bool ones_done = ones_scroller_.ScrollInDirection(diff_direction_);

  if (hundreds_lines_left_ > 0) {
    hundreds_lines_left_ -= 1;
    hundreds_scroller_.ScrollInDirection(diff_direction_);
  }

  if (tens_lines_left_ > 0) {
    tens_lines_left_ -= 1;
    tens_scroller_.ScrollInDirection(diff_direction_);
  }

  if (ones_done) {
    current_number_ += Delta();
    RecalculateImages();
  }
}

void ScrollManager::RecalculateImages() {
  ones_scroller_.SetNextImage(
      *number_2x_images[Ones(current_number_ + Delta())]);
  RecalculateHundredsLines();
  RecalculateTensLines();
}

void ScrollManager::RecalculateHundredsLines() {
  const uint8_t curentHundreds = Hundreds(current_number_);
  const int8_t nextNumber = current_number_ + Delta();
  const uint8_t nextHundreds = Hundreds(nextNumber);
  if (curentHundreds != nextHundreds) {
    hundreds_lines_left_ += Image2xHeight;
    hundreds_scroller_.SetNextImage(*number_2x_images[nextHundreds]);
  } else if (current_number_ < 0 && nextNumber >= 0) {
    hundreds_lines_left_ += Image2xHeight;
    hundreds_scroller_.SetNextImage(blank_2x);
  } else if (current_number_ >= 0 && nextNumber < 0) {
    hundreds_lines_left_ += Image2xHeight;
    hundreds_scroller_.SetNextImage(minus_2x);
  } else {
    hundreds_lines_left_ = 0;
  }
}

void ScrollManager::RecalculateTensLines() {
  const uint8_t nextTens = Tens(current_number_ + Delta());
  if (Tens(current_number_) != nextTens) {
    tens_lines_left_ += Image2xHeight;
    tens_scroller_.SetNextImage(*number_2x_images[nextTens]);
  } else {
    tens_lines_left_ = 0;
  }
}

int8_t ScrollManager::Delta() const {
  return diff_direction_ == ScrollDirection::Up ? -1 : 1;
}

uint8_t ScrollManager::Hundreds(int8_t number) {
  return abs(number) / 100;
}

uint8_t ScrollManager::Tens(int8_t number) {
  return (abs(number) % 100) / 10;
}

uint8_t ScrollManager::Ones(int8_t number) {
  return abs(number) % 10;
}
