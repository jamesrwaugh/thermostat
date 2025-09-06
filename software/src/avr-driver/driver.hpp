#include <Serial/HardwareSerial.h>
#include <driver_ds1307.h>
#include <etl/optional.h>
#include <tmp116.h>

#ifdef SIMULATED
#include "SimulatedGu7000/SimulatedGu7000.hpp"
#else
#include <Noritake_VFD_GU7000.h>
#endif

struct AvrDriverCallbacks;
struct ThermoButtonState;

struct AvrDrivers {
  AvrDrivers(const AvrDriverCallbacks& callbacks, void* userData);

  enum Gu7kWindowId {
    UpperRight = 1,
    LowerRight = 2,
    LowerLeft = 3,
  };

#ifdef SIMULATED
  SimulatedGu7000 Screen;
#else
  Noritake_VFD_GU7000 Screen;
#endif

  ds1307_handle_t Rtc;
  HardwareSerial& Serial_;
  TMP116 TempSensor;

  void Setup();
  void ReadInput();
  void ReadStateNow(ThermoButtonState* out) const;

 private:
  const AvrDriverCallbacks& Callbacks_;
  void* UserData_{nullptr};
  void SetupI2C();
  void SetupPins();
  void SetupScreen();
  void SetupInputTimer();
  uint8_t SetupRTC();
};

extern etl::optional<AvrDrivers> gDriver;