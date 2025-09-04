#include <Noritake_VFD_GU7000.h>
#include <Serial/HardwareSerial.h>
#include <driver_ds1307.h>
#include <etl/optional.h>
#include <tmp116.h>

struct AvrDriverCallbacks;
struct ThermoButtonState;

struct AvrDrivers {
  AvrDrivers(const AvrDriverCallbacks& callbacks, void* userData);

  enum Gu7kWindowId {
    UpperRight = 1,
    LowerRight = 2,
    LowerLeft = 3,
  };

  Noritake_VFD_GU7000 Screen;
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