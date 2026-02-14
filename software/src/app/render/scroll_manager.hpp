#pragma once

#include "scroller.hpp"

class ScollManager {
 public:
  ScollManager(Image2x& hundreds, Image2x& tens, Image2x& ones);

  void Calculate(int8_t old, int8_t theNew);
  bool IsFinished() const;
  void ApplyOnce();

 private:
  void RecalculateDigits();
  void RecalculateHundreds();
  void RecalculateTens();
  void RecalculateOnes();
  int8_t Delta() const;

  int8_t goal_number_{0};
  int8_t current_number_{0};
  uint8_t tens_lines_left_{0};
  uint8_t hundreds_lines_left_{0};
  ScrollDirection diff_direction_{ScrollDirection::Down};
  Scroller hundreds_scroller_;
  Scroller tens_scroller_;
  Scroller ones_scroller_;
};
