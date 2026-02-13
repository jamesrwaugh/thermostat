#pragma once

#include "scroller.hpp"

class ScollManager {
 public:
  ScollManager(Image2x& hundreds, Image2x& tens, Image2x& ones);

  void Calculate(int8_t old, int8_t theNew);
  bool Finished() const;
  void ApplyOnce();

 private:
  void RecalculateImages();
  void RecalculateHundredsLines();
  void RecalculateTensLines();
  int8_t Delta() const;
  static uint8_t Hundreds(int8_t number);
  static uint8_t Tens(int8_t number);
  static uint8_t Ones(int8_t number);

  int8_t goal_number_{0};
  int8_t current_number_{0};
  uint8_t tens_lines_left_{0};
  uint8_t hundreds_lines_left_{0};
  ScrollDirection diff_direction_{ScrollDirection::Down};
  ScrollerT hundreds_scroller_;
  ScrollerT tens_scroller_;
  ScrollerT ones_scroller_;
};
