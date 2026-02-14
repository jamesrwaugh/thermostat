#include <ftd2xx.h>
#include <libft4222.h>

#include <cstdio>
#include <cstdlib>
#include <simavr-toolbox/sim_i2c_smarter_base.hpp>

#pragma once

class SimFtdiGu7000 final : public SimAvrI2CSmarterComponent {
 public:
  SimFtdiGu7000(avr_t* avr)
      : SimAvrI2CSmarterComponent(avr, 0x50), handle_(OpenI2CDevice()) {
    if (!handle_) {
      printf("No I2C handle\n");
      std::abort();
    }
  }

  ~SimFtdiGu7000() {
    FT4222_UnInitialize(handle_);
    FT_Close(handle_);
  }

  virtual void OnDataReceived(const std::vector<uint8_t>& data) override {
    uint16_t written = 0;
    FT4222_I2CMaster_Write(handle_, 0x50, const_cast<uint8_t*>(data.data()),
                           data.size(), &written);
  }

  virtual void ResetStateMachine() override {}

 private:
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

  const FT_HANDLE handle_;
};
