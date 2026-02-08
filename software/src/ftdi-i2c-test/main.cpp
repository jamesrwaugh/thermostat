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

#include <ftdi.hpp>
#include <vector>

#include "GU7000/Noritake_VFD_GU7000.h"

// ==================================================== //

struct HandleCleanup {
  HandleCleanup(FT_HANDLE h) : handle_(h) {}

  ~HandleCleanup() {
    if (handle_) {
      CloseI2CDevice(handle_);
    }
  }

  FT_HANDLE const handle_;
};

struct AutoTwi {
  AutoTwi(FT_HANDLE h) : handle_(h) {
    current_message_.clear();
    current_message_.reserve(32);
  }

  ~AutoTwi() {
    uint16_t written = 0;
    FT4222_I2CMaster_Write(handle_, 0x50, current_message_.data(),
                           current_message_.size(), &written);
  }

  static std::vector<uint8> current_message_;
  FT_HANDLE const handle_;
};

std::vector<uint8> AutoTwi::current_message_;

// ==================================================== //

void initPort() {}

void writePort(const uint8_t data) {
  AutoTwi::current_message_.push_back(data);
}

void hardReset() {}

// ==================================================== //

int main(void) {
  FT_HANDLE handle = OpenI2CDevice();

  HandleCleanup c(handle);

  if (!handle) {
    return 1;
  }

  Noritake_VFD_GU7000 screen;

  {
    AutoTwi t(handle);
    screen.GU7000_init();
  }

  {
    AutoTwi t(handle);
    screen.GU7000_clearScreen();
    screen.print("Hello, world.");
  }

  {
    AutoTwi t(handle);
    screen.GU7000_setScrollMode(ScrollMode::VertScrollMode);
    screen.GU7000_scrollScreen(1, 2, 20, 1);
  }

  return 0;
}

// ==================================================== //
