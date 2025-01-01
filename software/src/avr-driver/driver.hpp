#include <Noritake_VFD_GU7000.h>
#include <Serial/HardwareSerial.h>
#include <driver_ds3231.h>
#include <tmp116.h>

struct AvrDriverCallbacks;

struct AvrDrivers {
  AvrDrivers(const AvrDriverCallbacks &callbacks);

  Noritake_VFD_GU7000 Screen;
  ds3231_handle_t Rtc;
  HardwareSerial &Serial_;
  TMP116 TempSensor;

  void Setup();

private:
  const AvrDriverCallbacks &Callbacks_;
  void SetupI2C();
  void SetupPins();
  void SetupScreen();
  void SetupTimer();
  uint8_t SetupRTC();
  void ReadInput();
};