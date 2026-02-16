#pragma once

#include <images.h>

#include "scroller.hpp"

class ScrollManager {
 public:
  ScrollManager(Image2x& hundreds, Image2x& tens, Image2x& ones);

  void Calculate(int16_t old, int16_t theNew);
  bool IsFinished() const;
  void ScrollOnce();
  uint16_t ScrollLinesLeft() const;

 private:
  void RecalculateDigits();
  void RecalculateHundreds();
  void RecalculateTens();
  void RecalculateOnes();
  int8_t Delta() const;

  int16_t goal_number_{0};
  int16_t current_number_{0};
  uint16_t total_lines_left_{0};
  uint8_t tens_lines_left_{0};
  uint8_t hundreds_lines_left_{0};
  ScrollDirection scroll_direction_{ScrollDirection::Down};
  Scroller hundreds_scroller_;
  Scroller tens_scroller_;
  Scroller ones_scroller_;
};
