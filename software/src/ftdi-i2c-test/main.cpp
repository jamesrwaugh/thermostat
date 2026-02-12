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
#include "scroller.hpp"

// ==================================================== //

FT_HANDLE gHandle = nullptr;

struct HandleCleanup {
  ~HandleCleanup() {
    if (gHandle) {
      CloseI2CDevice(gHandle);
    }
  }
};

struct AutoTwi {
  AutoTwi() {
    current_message_.reserve(32);
  }

  ~AutoTwi() {
    uint16_t written = 0;
    FT4222_I2CMaster_Write(gHandle, 0x50, current_message_.data(),
                           current_message_.size(), &written);
    current_message_.clear();
  }

  static std::vector<uint8> current_message_;
};

std::vector<uint8> AutoTwi::current_message_;

// ==================================================== //

void initPort() {}

void writePort(const uint8_t data) {
  AutoTwi::current_message_.push_back(data);
}

void hardReset() {}

// ==================================================== //

constexpr uint8_t Image1xWidth = 5;
constexpr uint8_t Image1xHeight = 7;

typedef const uint8_t Image1x[Image1xWidth];

const uint8_t gFireOneImageData[Image1xWidth] = {
  // clang-format off
    0b00011110,
    0b01111111,
    0b01111111,
    0b00011111,
    0b00000110,
  // clang-format on
};

class TwoDigitScroller {
  Image2x tens_;
  Image2x ones_;
};

class Renderer {
 public:
  static constexpr uint8_t CharacterWidth1x = 7;
  static constexpr uint8_t ScreenWidth = 112;
  static constexpr uint8_t TemperatureXPos = 0;
  static constexpr uint8_t HumidityXPos =
    TemperatureXPos + (3 * ImageWidth2x) + CharacterWidth1x + CharacterWidth1x;

  Renderer(Noritake_VFD_GU7000& s) : screen_(s) {}

  void DrawTemperature(int8_t number, char suffix) {
    AutoTwi t;

    const uint8_t hundredsOrMinusSpot = TemperatureXPos;
    const uint8_t posNumber = abs(number);

    if (number < 0) {
      screen_.print(hundredsOrMinusSpot, 0, "-");
    } else if (number >= 100) {
      screen_.GU7000_drawImage(hundredsOrMinusSpot, 0, ImageWidth2x,
                               ImageHeight2x,
                               *number_2x_images[posNumber / 100]);
    }

    DrawPositive2DigitNumber(TemperatureXPos + ImageWidth2x, posNumber % 100,
                             suffix);
  }

  void DrawHumidity(uint8_t number) {
    AutoTwi t;
    DrawPositive2DigitNumber(HumidityXPos, number, '%');
  }

  void DrawSetPoint(int8_t setPoint) {
    AutoTwi t;

    uint8_t setPointPos = ScreenWidth;

    if (setPoint < 0) {
      setPointPos -= CharacterWidth1x;
    }

    static const uint8_t posSetPoint = abs(setPoint);

    if (posSetPoint >= 100) {
      setPointPos -= 3 * CharacterWidth1x;
    } else if (posSetPoint >= 10) {
      setPointPos -= 2 * CharacterWidth1x;
    } else {
      setPointPos -= 1 * CharacterWidth1x;
    }

    screen_.print(setPointPos, 0, setPoint, 10);
  }

  void DrawHeatingStatus(bool active) {
    AutoTwi t;

    const uint8_t imagePosition = ScreenWidth - Image1xWidth - 1;

    if (active) {
      screen_.print(imagePosition - (CharacterWidth1x * 2) - 2, 8, "ON");
    }

    Draw1xImage(imagePosition, true, gFireOneImageData);
  }

 private:
  void DrawPositive2DigitNumber(uint8_t xPos, uint8_t number, char suffix) {
    const uint8_t tensSpot = xPos;
    const uint8_t onesSpot = tensSpot + ImageWidth2x + 1;
    const uint8_t suffixSpot = onesSpot + ImageWidth2x + 1;

    screen_.GU7000_drawImage(tensSpot, 0, ImageWidth2x, ImageHeight2x,
                             *number_2x_images[number / 10]);

    screen_.GU7000_drawImage(onesSpot, 0, ImageWidth2x, ImageHeight2x,
                             *number_2x_images[number % 10]);

    screen_.print(suffixSpot, 0, suffix);
  }

  void Draw1xImage(uint8_t xPositionDots, bool bottom, Image1x image) {
    screen_.GU7000_drawImage(xPositionDots, Image1xHeight, Image1xWidth,
                             bottom ? 8 : 0, image);
  }

  Noritake_VFD_GU7000& screen_;
  Image2x temperature_tens_;
  Image2x temperature_ones_;
  Image2x humidity_tens_;
  Image2x humidity_ones_;
};

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

  r.DrawTemperature(-2, 'F');
  r.DrawHumidity(1);
  r.DrawSetPoint(-32);
  r.DrawHeatingStatus(false);

  // {
  //   AutoTwi t;
  //   screen.print(10 - 5 - 1, 0, "-");
  //   screen.GU7000_drawImage(10, 0, ImageWidth2x, ImageHeight2x, image_7_2x);
  //   screen.GU7000_drawImage(10 + ImageWidth2x + 1, 0, ImageWidth2x,
  //                           ImageHeight2x, image_7_2x);
  //   screen.GU7000_drawImage(10 + ImageWidth2x + 1 + ImageWidth2x + 1, 0,
  //                           ImageWidth2x, ImageHeight2x, image_7_2x);
  //   screen.print(10 + ImageWidth2x + 1 + ImageWidth2x + 1 + ImageWidth2x + 2,
  //   0,
  //                "F");
  // }

  // for (int i = 0; i < ImageHeight2x; ++i) {
  //   s.ScrollOneUp();
  //   s.Draw();
  //   usleep(20000);
  // }

  // ScrollerT s(screen, 20, 5, number_2x_images);

  // {
  //   AutoTwi t;
  //   screen.print(50, 0, "  ");
  //   screen.print(50, 0, s.ScrolledCount(), 10);
  // }

  // {
  //   AutoTwi t;
  //   s.Draw();
  // }

  // for (int i = 0; i < (ImageHeight2x * 2) + 8; ++i) {
  //   AutoTwi t;
  //   s.ScrollDownOneLine();
  //   s.Draw();
  //   PrintNumberScrolled(handle, screen, s);
  //   usleep(20000 + 1300 * i);
  // }

  // for (int i = 0; i < 12; ++i) {
  //   AutoTwi t;
  //   s.ScrollDownOneLine();
  //   s.Draw();
  //   PrintNumberScrolled(handle, screen, s);
  //   usleep(std::min(2000, 30000 - i * 500));
  // }

  // usleep(500000);

  // for (int i = 0; i < ImageHeight2x; ++i) {
  //   AutoTwi t;
  //   s.ScollUpOneLine();
  //   s.Draw();
  //   PrintNumberScrolled(handle, screen, s);
  //   usleep(20000 + 1300 * i);
  // }

  // usleep(500000);

  // for (int i = 0; i < (ImageHeight2x * 3) + 2; ++i) {
  //   AutoTwi t;
  //   s.ScrollDownOneLine();
  //   s.Draw();
  //   PrintNumberScrolled(handle, screen, s);
  //   usleep(std::min(40000 - 400 * i, 20000));
  // }

  // for (int i = 0; i < (ImageHeight2x * 5); ++i) {
  //   AutoTwi t;
  //   s.ScollUpOneLine();
  //   s.Draw();
  //   PrintNumberScrolled(handle, screen, s);
  //   usleep(std::min(30000 - 400 * i, 20000));
  // }

  return 0;
}

// ==================================================== //
