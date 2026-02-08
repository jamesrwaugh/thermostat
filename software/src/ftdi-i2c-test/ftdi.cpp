#include "ftdi.hpp"

#include <stdio.h>

FT_HANDLE OpenI2CDevice() {
  FT_STATUS status = FT_OK;

  DWORD numDevices = 0;
  status = FT_CreateDeviceInfoList(&numDevices);
  if (status != FT_OK) {
    printf("FT_CreateDeviceInfoList failed (error code %d)\n", (int)status);
    return NULL;
  }

  if (numDevices != 1) {
    printf("FT_CreateDeviceInfoList expected 1 device");
    return NULL;
  }

  FT_DEVICE_LIST_INFO_NODE device;

  /* Populate the list of info nodes */
  status = FT_GetDeviceInfoList(&device, &numDevices);
  if (status != FT_OK) {
    printf("FT_GetDeviceInfoList failed (error code %d)\n", (int)status);
    return NULL;
  }

  if (device.Type != FT_DEVICE_4222H_3) {
    printf("Expected device to be opened in mode 3 for I2C");
    return NULL;
  }

  FT_HANDLE handle;
  status =
    FT_OpenEx((PVOID)(uintptr_t)device.LocId, FT_OPEN_BY_LOCATION, &handle);
  if (status != FT_OK) {
    printf("FT_OpenEx failed (error %d)\n", (int)status);
    return NULL;
  }

  status = FT4222_I2CMaster_Init(handle, 250);
  if (status != FT_OK) {
    printf("FT4222_I2CMaster_Init failed (error %d)\n", (int)status);
    return NULL;
  }

  return handle;
}

void CloseI2CDevice(FT_HANDLE handle) {
  FT4222_UnInitialize(handle);
  FT_Close(handle);
}

void WriteI2CBytes(FT_HANDLE handle, uint8_t slaveAddress, uint8_t* bytes,
                   uint8_t numBytes) {
  uint16_t bytesTransferred = 0;
  FT_STATUS status = FT4222_I2CMaster_Write(handle, slaveAddress, bytes,
                                            numBytes, &bytesTransferred);
  if (status != FT_OK) {
    printf("FT4222_I2CMaster_Write failed (error %d)\n", (int)status);
    return;
  }
}

void ReadI2CBytes(FT_HANDLE handle, uint8_t slaveAddress, uint8_t slaveRegister,
                  uint8_t* bytes, uint8_t numBytes) {
  uint16_t bytesTransferred = 0;

  uint8_t registerBuffer[1] = {slaveRegister};

  FT4222_I2CMaster_WriteEx(handle, slaveAddress, START, registerBuffer, 1,
                           &bytesTransferred);

  FT4222_I2CMaster_ReadEx(handle, slaveAddress, Repeated_START | STOP, bytes,
                          numBytes, &bytesTransferred);
}
