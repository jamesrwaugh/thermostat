#include "scroll_manager.hpp"

#include "digit_ops.hpp"

ScollManager::ScollManager(Image2x& hundreds, Image2x& tens, Image2x& ones)
    : hundreds_scroller_(hundreds, blank_2x, blank_2x),
      tens_scroller_(tens, blank_2x, blank_2x),
      ones_scroller_(ones, blank_2x, blank_2x) {}

void ScollManager::Calculate(int8_t old, int8_t theNew) {
  diff_direction_ =
    (theNew < old) ? ScrollDirection::Up : ScrollDirection::Down;
  current_number_ = old;
  goal_number_ = theNew;
  RecalculateDigits();
}

bool ScollManager::IsFinished() const {
  return current_number_ == goal_number_;
}

void ScollManager::ScrollOnce() {
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
    RecalculateDigits();
  }
}

void ScollManager::RecalculateDigits() {
  RecalculateHundreds();
  RecalculateTens();
  RecalculateOnes();
}

void ScollManager::RecalculateHundreds() {
  const int8_t nextNumber = current_number_ + Delta();
  const uint8_t nowHundreds = Hundreds(current_number_);
  const uint8_t nextHundreds = Hundreds(nextNumber);

  const auto& curentHundredsImage = number_2x_images[nowHundreds];

  if (nowHundreds != nextHundreds) {
    hundreds_lines_left_ += ImageHeight2x;
    hundreds_scroller_.SetImages(curentHundredsImage,
                                 number_2x_images[nextHundreds]);
  } else if (current_number_ < 0 && nextNumber >= 0) {
    hundreds_lines_left_ += ImageHeight2x;
    hundreds_scroller_.SetImages(curentHundredsImage, &blank_2x);
  } else if (current_number_ >= 0 && nextNumber < 0) {
    hundreds_lines_left_ += ImageHeight2x;
    hundreds_scroller_.SetImages(curentHundredsImage, &minus_2x);
  } else {
    hundreds_lines_left_ = 0;
  }
}

void ScollManager::RecalculateTens() {
  const uint8_t nowTens = Tens(current_number_);
  const uint8_t nextTens = Tens(current_number_ + Delta());

  if (nowTens != nextTens) {
    tens_lines_left_ += ImageHeight2x;
    tens_scroller_.SetImages(number_2x_images[nowTens],
                             number_2x_images[nextTens]);
  } else {
    tens_lines_left_ = 0;
  }
}

void ScollManager::RecalculateOnes() {
  const uint8_t nowOnes = Ones(current_number_);
  const uint8_t nextOnes = Ones(current_number_ + Delta());

  ones_scroller_.SetImages(number_2x_images[nowOnes],
                           number_2x_images[nextOnes]);
}

int8_t ScollManager::Delta() const {
  return diff_direction_ == ScrollDirection::Up ? -1 : 1;
}
