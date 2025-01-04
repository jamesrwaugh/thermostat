#pragma once

#include <stdint.h>

class TMP116 {
 public:
  void init();
  uint16_t read_temp() const;
};
