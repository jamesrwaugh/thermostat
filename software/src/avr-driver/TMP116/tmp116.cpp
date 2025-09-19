#include "tmp116.h"

#include <stdint.h>
#include <twi_master.h>

constexpr uint8_t TMP_IIC_ADDR = 0x48;  // Right shifted, R/W on bit 0
constexpr uint8_t TMP_IIC_PTR_TEMP = 0x00;
constexpr uint8_t TMP_IIC_PTR_CFG = 0x01;

void TMP116::Init() {}

uint16_t TMP116::ReadTempC() const {
  return 0;
}