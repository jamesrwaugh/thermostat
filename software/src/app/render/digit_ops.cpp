#include "digit_ops.hpp"

#include <stdlib.h>

uint8_t Hundreds(int8_t number) {
  return abs(number) / 100;
}

uint8_t Tens(int8_t number) {
  return (abs(number) % 100) / 10;
}

uint8_t Ones(int8_t number) {
  return abs(number) % 10;
}
