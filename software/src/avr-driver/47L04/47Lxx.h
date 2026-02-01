/*
        SerialRAM.h
        Very simple class to interface with Microchip's 4K/16K I2C Serial EERAM
   (47L04, 47C04, 47L16 and 47C16) chips

        This example code is licensed under CC BY 4.0.
        Please see https://creativecommons.org/licenses/by/4.0/

        modified 6th August 2017
        by Tony Pottier
        https://idyl.io

*/
#ifndef _SerialRAM_h
#define _SerialRAM_h

#include <stdint.h>

class SerialRAM {
 private:
  uint8_t i2c_data_address_{0};
  uint8_t i2c_control_address_{0};
  uint8_t readControlRegister() const;

 public:
  void begin(const uint8_t A2 = 0, const uint8_t A1 = 0);
  uint8_t write(const uint16_t address, const uint8_t value) const;
  uint8_t writeRaw(const uint8_t* values, const uint16_t size) const;
  uint8_t read(const uint16_t address) const;
  void setAutoStore(const bool value) const;
  bool getAutoStore();
  uint8_t write(const uint16_t address, const uint8_t* values,
                const uint16_t size) const;
  void read(const uint16_t address, uint8_t* values, const uint16_t size) const;
};

#endif
