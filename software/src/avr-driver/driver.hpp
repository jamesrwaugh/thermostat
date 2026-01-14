#include <Noritake_VFD_GU7000.h>
#include <Serial/HardwareSerial.h>
#include <driver_ds1307.h>
#include <etl/optional.h>
#include <tmp116.h>

#include "ThermoSaveData_bp.h"
#include "data_types.hpp"

struct AvrDriverCallbacks;
struct ThermoButtonState;

struct AvrDrivers {
  AvrDrivers();

  Noritake_VFD_GU7000 Screen;
  ds1307_handle_t Rtc;
  HardwareSerial& Serial_;
  TMP116 TempSensor;

  void Setup();
  int8_t ReadInput();
  void ReadStateNow(ThermoButtonState* out) const;
  void RelayOn(Relay r) const;
  void RelayOff(Relay r) const;
  bool SaveData(const ThermoSaveData& data) const;
  bool LoadData(ThermoSaveData& data) const;

 private:
  void SetupI2C();
  void SetupPins();
  void SetupScreen();
  void SetupInputTimer();
  uint8_t SetupRTC();
  void SetupTemp();
  void SetupSerial();
  bool WriteFlash(uint16_t address, const uint8_t* data, uint8_t length) const;
  bool ReadFlash(uint16_t address, uint8_t* buffer, uint8_t maxLength) const;
};

extern etl::optional<AvrDrivers> gDriver;