#include <stdint.h>

void initPort();
void writePort(const uint8_t data, const uint8_t busyPin);
void hardReset();
