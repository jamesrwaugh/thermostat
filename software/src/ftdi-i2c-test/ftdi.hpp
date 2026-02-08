#include <ftd2xx.h>
#include <libft4222.h>

FT_HANDLE OpenI2CDevice();

void CloseI2CDevice(FT_HANDLE handle);

void WriteI2CBytes(FT_HANDLE handle, uint8_t slaveAddress, uint8_t* bytes,
                   uint8_t numBytes);

void ReadI2CBytes(FT_HANDLE handle, uint8_t slaveAddress, uint8_t slaveRegister,
                  uint8_t* bytes, uint8_t numBytes);
