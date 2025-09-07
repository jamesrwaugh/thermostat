#include "tmp116.h"

#include <stdint.h>
#include <twi_master.h>

constexpr uint8_t TMP_IIC_ADDR = 0x48;  // Right shifted, R/W on bit 0
constexpr uint8_t TMP_IIC_PTR_TEMP = 0x00;
constexpr uint8_t TMP_IIC_PTR_CFG = 0x01;
constexpr uint32_t TEMP_RESOLUTION_INT = 78125;  // 0.0078125×10^7
constexpr uint64_t START = 10'000'000;
constexpr uint64_t END = 100'000'000'000'000;

void TMP116::init() {
  uint8_t start_data[] = {TMP_IIC_PTR_CFG};
  tw_master_transmit(TMP_IIC_ADDR, start_data, sizeof(start_data), true);

  // 11:10 MOD[1:0]  =  00: Continuous conversion (CC)
  // 9:7   CONV[2:0] = 100: 1s Conversion
  // 6:5   AVG[1:0]  =  01: 8 samples averaged
  uint8_t cfg_data[] = {0b0000'0010, 0b0010'0000};
  tw_master_transmit(TMP_IIC_ADDR, cfg_data, sizeof(cfg_data), false);
}

uint16_t TMP116::read_temp() const {
  uint8_t start_data[] = {TMP_IIC_PTR_TEMP};
  tw_master_transmit(TMP_IIC_ADDR, start_data, sizeof(start_data), true);

  uint8_t temp_data[] = {0, 0};
  tw_master_receive(TMP_IIC_ADDR, temp_data, sizeof(temp_data));

  int64_t temp = ((temp_data[0] << 8) | temp_data[1]);

  if (temp < 0) {
    return 0;
  }

  temp *= START;
  temp *= TEMP_RESOLUTION_INT;
  temp /= END;

  return temp;
}