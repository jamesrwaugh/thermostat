#include <stdint.h>

extern "C" uint32_t RustAdd2(uint32_t a, uint32_t b);

int main() {
  // Hello
  return RustAdd2(1, 2);
}