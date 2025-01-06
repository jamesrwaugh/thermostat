#include <Noritake_VFD_GU7000.h>
#include <Serial/HardwareSerial.h>
#include <driver_ds3231.h>
#include <etl/optional.h>
#include <tmp116.h>

struct AvrDriverCallbacks;

struct AvrDrivers {
  AvrDrivers(const AvrDriverCallbacks &callbacks, void *userData);

  Noritake_VFD_GU7000 Screen;
  ds3231_handle_t Rtc;
  HardwareSerial &Serial_;
  TMP116 TempSensor;

  void Setup();
  void ReadInput();

 private:
  const AvrDriverCallbacks &Callbacks_;
  void *UserData_{nullptr};
  void SetupI2C();
  void SetupPins();
  void SetupScreen();
  void SetupInputTimer();
  void SetupSleep();
  uint8_t SetupRTC();
};

extern etl::optional<AvrDrivers> gDriver;