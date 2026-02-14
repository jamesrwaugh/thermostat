#include "scroll_manager.hpp"

#include <stdlib.h>

#include "digit_ops.hpp"

ScrollManager::ScrollManager(Image2x& hundreds, Image2x& tens, Image2x& ones)
    : hundreds_scroller_(hundreds, Image2xId::Blank, Image2xId::Blank),
      tens_scroller_(tens, Image2xId::Blank, Image2xId::Blank),
      ones_scroller_(ones, Image2xId::Blank, Image2xId::Blank) {}

void ScrollManager::Calculate(int8_t old, int8_t theNew) {
  current_number_ = old;
  goal_number_ = theNew;
  remaining_lines_ = abs(goal_number_ - current_number_) * Image2xHeight;
  if (current_number_ != goal_number_) {
    scroll_direction_ =
        (theNew < old) ? ScrollDirection::Up : ScrollDirection::Down;
    RecalculateDigits();
  }
}

bool ScrollManager::IsFinished() const {
  return current_number_ == goal_number_;
}

uint16_t ScrollManager::RemainingLines() const {
  return remaining_lines_;
}

void ScrollManager::ScrollOnce() {
  bool ones_done = ones_scroller_.ScrollInDirection(scroll_direction_);

  if (remaining_lines_ >= 0) {
    remaining_lines_ -= 1;
  }

  if (hundreds_lines_left_ > 0) {
    hundreds_lines_left_ -= 1;
    hundreds_scroller_.ScrollInDirection(scroll_direction_);
  }

  if (tens_lines_left_ > 0) {
    tens_lines_left_ -= 1;
    tens_scroller_.ScrollInDirection(scroll_direction_);
  }

  if (ones_done) {
    current_number_ += Delta();
    RecalculateDigits();
  }
}

void ScrollManager::RecalculateDigits() {
  RecalculateHundreds();
  RecalculateTens();
  RecalculateOnes();
}

void ScrollManager::RecalculateHundreds() {
  const int8_t nextNumber = current_number_ + Delta();
  const uint8_t nowHundreds = Hundreds(current_number_);
  const uint8_t nextHundreds = Hundreds(nextNumber);

  const auto& curentHundredsImage = static_cast<Image2xId>(nowHundreds);

  if (nowHundreds != nextHundreds) {
    hundreds_lines_left_ += Image2xHeight;
    hundreds_scroller_.SetImages(curentHundredsImage,
                                 static_cast<Image2xId>(nextHundreds));
  } else if (current_number_ < 0 && nextNumber >= 0) {
    hundreds_lines_left_ += Image2xHeight;
    hundreds_scroller_.SetImages(curentHundredsImage, Image2xId::Blank);
  } else if (current_number_ >= 0 && nextNumber < 0) {
    hundreds_lines_left_ += Image2xHeight;
    hundreds_scroller_.SetImages(curentHundredsImage, Image2xId::Minus);
  } else {
    hundreds_lines_left_ = 0;
  }
}

void ScrollManager::RecalculateTens() {
  const uint8_t nowTens = Tens(current_number_);
  const uint8_t nextTens = Tens(current_number_ + Delta());

  if (nowTens != nextTens) {
    tens_lines_left_ += Image2xHeight;
    tens_scroller_.SetImages(static_cast<Image2xId>(nowTens),
                             static_cast<Image2xId>(nextTens));
  } else {
    tens_lines_left_ = 0;
  }
}

void ScrollManager::RecalculateOnes() {
  const uint8_t nowOnes = Ones(current_number_);
  const uint8_t nextOnes = Ones(current_number_ + Delta());

  ones_scroller_.SetImages(static_cast<Image2xId>(nowOnes),
                           static_cast<Image2xId>(nextOnes));
}

int8_t ScrollManager::Delta() const {
  return scroll_direction_ == ScrollDirection::Up ? -1 : 1;
}
