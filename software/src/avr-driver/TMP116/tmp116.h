#pragma once

#include <stdint.h>

class TMP116 {
 public:
  void Init();
  uint16_t ReadTempC() const;
};
