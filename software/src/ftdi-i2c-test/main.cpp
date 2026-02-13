/* Minimal program to link to LibFT4222.
 * Displays library and chip version numbers.
 *
 * Windows instructions:
 *  1. Copy ftd2xx.h and ftd2xx.lib from driver package to current directory.
 *  2. Build with MSVC:   cl i2cm.c LibFT4222.lib ftd2xx.lib
 *       or
 *     Build with MinGW:  gcc i2cm.c LibFT4222.lib ftd2xx.lib
 *  3. get-version.exe
 *
 * Linux instructions:
 *  1. Ensure libft4222.so is in the library search path (e.g. /usr/local/lib)
 *  2. Ensure libft4222.h, ftd2xx.h and WinTypes.h are in the Include search
 *     path (e.g. /usr/local/include).
 *  3. cc get-version.c -lft4222 -Wl,-rpath,/usr/local/lib
 *  4. sudo ./a.out
 *
 * Mac instructions:
 *  1. Ensure libft4222.dylib is in the library search path (e.g.
 * /usr/local/lib)
 *  2. Ensure libft4222.h, ftd2xx.h and WinTypes.h are in the Include search
 *     path (e.g. /usr/local/include).
 *  3. cc get-version.c -lft4222 -Wl,-L/usr/local/lib
 *  4. ./a.out
 */
#include <ftd2xx.h>
#include <libft4222.h>
#include <stdlib.h>
#include <unistd.h>

#include <ftdi.hpp>
#include <vector>

#include "GU7000/Noritake_VFD_GU7000.h"
#include "autotwi.hpp"
#include "renderer.hpp"
#include "scroll_manager.hpp"

// ==================================================== //

FT_HANDLE gHandle = nullptr;

std::vector<uint8> AutoTwi::current_message_;

struct HandleCleanup {
  ~HandleCleanup() {
    if (gHandle) {
      CloseI2CDevice(gHandle);
    }
  }
};

void initPort() {}

void writePort(const uint8_t data) {
  AutoTwi::current_message_.push_back(data);
}

void hardReset() {}

int main(void) {
  gHandle = OpenI2CDevice();

  HandleCleanup c;

  if (!gHandle) {
    return 1;
  }

  Noritake_VFD_GU7000 screen;

  {
    AutoTwi t;
    screen.GU7000_init();
    screen.GU7000_clearScreen();
    screen.GU7000_setScreenBrightness(10);
  }

  Renderer r(screen);

  int8_t oldTemp = -2;
  int8_t newTemp = 40;

  r.InitializeImages(oldTemp, 2);

  r.DrawTemperature('F');
  r.DrawHumidity();
  r.DrawSetPoint(-32);
  r.DrawHeatingStatus(true);

  auto& images = r.GetImages();

  ScollManager m(images.temperature_hundreds_or_minus_,
                 images.temperature_tens_, images.temperature_ones_);

  m.Calculate(oldTemp, newTemp);

  // unsigned count = 0;
  // while (!m.Finished()) {
  //   m.ApplyOnce();
  //   r.DrawTemperature('F');
  //   usleep(std::clamp(50000u - (count * 500), 25000u, 50000u));
  //   count += 1;
  // }

  while (1) {
    r.DrawHeatingStatus(HeatModeT::Heating, true);

    if (!m.Finished()) {
      m.ApplyOnce();
      r.DrawTemperature('F');
    }

    usleep(75000);
  }

  return 0;
}

// ==================================================== //
