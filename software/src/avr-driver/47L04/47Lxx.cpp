/*
        SerialRAM.cpp
        Very simple class to interface with Microchip's 4K/16K I2C Serial EERAM
   (47L04, 47C04, 47L16 and 47C16) chips

        This example code is licensed under CC BY 4.0.
        Please see https://creativecommons.org/licenses/by/4.0/

        modified 6th August 2017
        by Tony Pottier
        https://idyl.io

*/

#include "47Lxx.h"

#include <I2C/twi/twi_master.h>
#include <stdint.h>

typedef union {
  uint16_t a16;
  uint8_t a8[2];
} address16b;

///< summary>
///	Initialize the RAM chip with the given A0 and A1 values.
///	<param name="A0">A0 value (logic 0 or 1) of the RAM chip you want to
///address. Default value
/// 0.</param> 	<param name="A1">A1 value (logic 0 or 1) of the RAM chip you
/// want to address. Default value 0.</param>
///</summary>
void SerialRAM::begin(const uint8_t A2, const uint8_t A1) {
  // build mask
  uint8_t mask = (A2 << 1) | (A1);
  mask <<= 2;

  // save registers addresses
  SRamAddr_ = 0x50 | mask;
  ControlAddr_ = 0x18 | mask;
}

///< summary>
///	Write the given byte "value" at the 16 bit address "address".
///		47x16 chips valid addresses range from 0x0000 to 0x07FF
///		47x04 chips valid addresses range from 0x0000 to 0x01FF
///		<param name="address">16 bit address</param>
///		<param name="value">value (byte) to be written</param>
///		<returns>0:success, 1:data too long to fit in transmit buffer, 2
///: received NACK on
/// transmit of address, 3 : received NACK on transmit of data, 4 : other error
/// </returns>
///</summary>
uint8_t SerialRAM::write(const uint16_t address, const uint8_t value) {
  address16b a;
  a.a16 = address;
  uint8_t data[3] = {a.a8[1], a.a8[0], value};
  return tw_master_transmit(SRamAddr_, data, 3, false);
}

uint8_t SerialRAM::writeRaw(const uint8_t* values, const uint16_t size) {
  for (uint16_t i = 0; i < size; ++i) {
    tw_write(values[i]);
  }
  return 0;
}

///< summary>
///	Read the byte "value" located at the 16 bit address "address".
///		47x16 chips valid addresses range from 0x0000 to 0x07FF
///		47x04 chips valid addresses range from 0x0000 to 0x01FF
///		<param name="address">16 bit address</param>
///		<returns>value (byte) read at the address</returns>
///</summary>
uint8_t SerialRAM::read(const uint16_t address) {
  address16b a;
  a.a16 = address;
  uint8_t data[2] = {a.a8[1], a.a8[0]};
  uint8_t item = 0;
  tw_master_transmit(SRamAddr_, data, sizeof(data), true);
  tw_master_receive(SRamAddr_, &item, 1);
  return item;
}

uint8_t SerialRAM::readControlRegister() {
  uint8_t buffer = 0x80;
  tw_master_transmit_one(ControlAddr_, 0x00, true);
  tw_master_receive(ControlAddr_, &buffer, 1);
  return buffer;
}

///< summary>
///	De/Activate the "AutoStore" to EEPROM functionnality of the RAM when
///power is lost. 		<param name="value">Set to true to activate, false
///otherwise</param>
///</summary>
void SerialRAM::setAutoStore(const bool value) {
  uint8_t buffer = readControlRegister();
  buffer = value ? buffer | 0x02 : buffer & 0xfd;
  tw_master_transmit_one(ControlAddr_, buffer, true);
}

///< summary>
///	De/Activate the "AutoStore" to EEPROM functionality of the RAM when
///power is lost. 		<returns>true of auto store is active</return>
///</summary>
bool SerialRAM::getAutoStore() {
  uint8_t buffer = readControlRegister();
  return buffer & 0x02;
}

///< summary>
///	Write the array of bytes "values" at the 16 bit address "address".
///		47x16 chips valid addresses range from 0x0000 to 0x07FF
///		47x04 chips valid addresses range from 0x0000 to 0x01FF
///		<param name="address">16 bit address</param>
///		<param name="values">values (bytes) to be written</param>
///		<returns>0:success, 1:data too long to fit in transmit buffer, 2
///: received NACK on
/// transmit of address, 3 : received NACK on transmit of data, 4 : other error
/// </returns>
///</summary>
uint8_t SerialRAM::write(const uint16_t address, const uint8_t* values,
                         const uint16_t size) {
  ret_code_t error = 0;
  address16b a;
  a.a16 = address;

  error |= tw_master_setup_transmit(SRamAddr_);

  error |= tw_write(a.a8[1]);
  error |= tw_write(a.a8[0]);

  for (uint16_t i = 0; i < size; ++i) {
    error |= tw_write(values[i]);
  }

  tw_master_end_transmit();

  return error;
}

///< summary>
///	Read "size" number of bytes into "values" array located at the 16 bit
///address "address". 		Make sure values is big enough to contain all data or a
///segfault will occur. 		<param name="address">16 bit starting address of the
///data</param> 		<param name="values">array to be used to store the data</param>
///		<param name="size">number of bytes to retrieve</param>
///</summary>
void SerialRAM::read(const uint16_t address, uint8_t* values,
                     const uint16_t size) {
  address16b a;
  a.a16 = address;
  uint8_t data[2] = {a.a8[1], a.a8[0]};
  tw_master_transmit(SRamAddr_, data, sizeof(data), true);
  tw_master_receive(SRamAddr_, values, size);
}
