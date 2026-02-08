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

constexpr uint8_t ImageWidth = 5;
constexpr uint8_t ImageHeight = 7;

typedef const uint8_t Image[ImageWidth];

const uint8_t gFireOneImageData[ImageWidth] = {
  // clang-format off
    0b00011110,
    0b11111111,
    0b11111111,
    0b01111111,
    0b00000110,
  // clang-format on
};

// void DrawImage(uint8_t xPositionDots, bool bottom, Image image) {
//   AutoTwi t;
//   DriverGetScreenHandle().GU7000_drawImage(xPositionDots, ImageHeight,
//                                            ImageWidth, bottom ? 8 : 0,
//                                            image);
// }

/* 10x16 image "Untitled" */
static const uint8_t image_Untitled2[] = {
  0xc0, 0x01, 0x40, 0x03, 0x40, 0x0c, 0x40, 0x10, 0x40, 0x20,
  0x20, 0xc0, 0x21, 0x00, 0x36, 0x00, 0x1c, 0x00, 0x80, 0x01};

/* 10x16 image "Untitled" */

constexpr uint8_t ImageWidth2x = 10;
constexpr uint8_t ImageWidth2xSize = ImageWidth2x * 2;
constexpr uint8_t ImageHeight2x = 16;

typedef const uint8_t Image2x[ImageWidth2x];

static const uint8_t image_Untitled[ImageWidth2xSize] = {
  // clang-format off
  0b11000000,  // Top
  0b00000001,  // Bottom
  0b01000000,  // Top
  0b00000011,  // Bottom
  0b01000000, 
  0b00001100,
  0b01000000, 
  0b00010000, 
  0b01000000, 
  0b00100000,
  0b00100000, 
  0b11000000, 
  0b00100001, 
  0b00000000, 
  0b00110110, 
  0b00000000,
  0b00011100, 
  0b00000000, 
  0b10000000, 
  0b00000001,
  // clang-format on
};

void GetNthLine(const Image2x& image, uint8_t number) {
  uint8_t buffer[8];
  if (number < 8) {
    uint8_t relativeNumber = number - 8;
    for (uint8_t i = 0; i < ImageWidth2x; ++i) {
      buffer[i] = image[2 * i] & (1 << relativeNumber);
    }
    // Even bytes
  } else {
    // Odd bytes
  }
}

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
    // screen.GU7000_cursorOn();
  }

  // {
  //   AutoTwi t(handle);
  //   screen.GU7000_setScrollMode(ScrollMode::VertScrollMode);
  //   // screen.GU7000_defineWindow(1, 0, 0, 5 * 14, 8);
  //   // screen.GU7000_selectWindow(1);
  //   // screen.GU7000_setHorizScrollSpeed(1);
  //   screen.print("NewTextHere");
  // }

  {
    AutoTwi t(handle);
    screen.GU7000_drawImage(10, 16, image_Untitled);
  }

  // {
  //   AutoTwi t(handle);
  //   screen.GU7000_setFontSize(2, 2, false);
  //   screen.print("77");
  // }

  // {
  //   AutoTwi t(handle);
  //   screen.GU7000_drawImage(10, ImageHeight, ImageWidth, true ? 8 : 0,
  //                           gFireOneImageData);
  // }

  return 0;
}

// ==================================================== //
