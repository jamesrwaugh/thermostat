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

#include <cstring>
#include <ftdi.hpp>
#include <vector>

#include "GU7000/Noritake_VFD_GU7000.h"
#include "scroller.hpp"
#include "temperature.hpp"

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

class Renderer {
 public:
  static constexpr uint8_t CharacterWidth1x = 7;
  static constexpr uint8_t ScreenWidth = 112;
  static constexpr uint8_t TemperatureXPos = 0;
  static constexpr uint8_t HumidityXPos =
    TemperatureXPos + (3 * ImageWidth2x) + CharacterWidth1x + CharacterWidth1x;

  struct Images {
    Image2x temperature_hundreds_or_minus_;
    Image2x temperature_tens_;
    Image2x temperature_ones_;
    Image2x humidity_tens_;
    Image2x humidity_ones_;
  };

  Renderer(Noritake_VFD_GU7000& s) : screen_(s) {}

  void InitializeImages(int8_t temperature, uint8_t humidity) {
    const uint8_t posTemp = abs(temperature);

    const Image2x* hundreds_image = &blank_2x;

    if (temperature < 0) {
      hundreds_image = &minus_2x;
    } else if (temperature >= 100) {
      hundreds_image = number_2x_images[posTemp / 100];
    } else {
      hundreds_image = &blank_2x;
    }

    memcpy(&images_.temperature_hundreds_or_minus_, hundreds_image,
           sizeof(Image2x));
    memcpy(&images_.temperature_tens_, number_2x_images[(posTemp % 100) / 10],
           sizeof(Image2x));
    memcpy(&images_.temperature_ones_, number_2x_images[posTemp % 10],
           sizeof(Image2x));
    memcpy(&images_.humidity_tens_, number_2x_images[humidity / 10],
           sizeof(Image2x));
    memcpy(&images_.humidity_ones_, number_2x_images[humidity % 10],
           sizeof(Image2x));
  }

  void DrawTemperature(char suffix) {
    AutoTwi t;

    screen_.GU7000_drawImage(TemperatureXPos, 0, ImageWidth2x, ImageHeight2x,
                             images_.temperature_hundreds_or_minus_);

    DrawPositive2DigitNumber(TemperatureXPos + ImageWidth2x,
                             images_.temperature_tens_,
                             images_.temperature_ones_, suffix);
  }

  void DrawHumidity() {
    AutoTwi t;
    DrawPositive2DigitNumber(HumidityXPos, images_.humidity_tens_,
                             images_.humidity_ones_, '%');
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

  Images& GetImages() {
    return images_;
  }

 private:
  void DrawPositive2DigitNumber(uint8_t xPos,
                                const Image2x& tens,
                                const Image2x& ones,
                                char suffix) {
    const uint8_t tensSpot = xPos;
    const uint8_t onesSpot = tensSpot + ImageWidth2x + 1;
    const uint8_t suffixSpot = onesSpot + ImageWidth2x + 1;

    screen_.GU7000_drawImage(tensSpot, 0, ImageWidth2x, ImageHeight2x, tens);

    screen_.GU7000_drawImage(onesSpot, 0, ImageWidth2x, ImageHeight2x, ones);

    screen_.print(suffixSpot, 0, suffix);
  }

  void Draw1xImage(uint8_t xPositionDots, bool bottom, Image1x image) {
    screen_.GU7000_drawImage(xPositionDots, Image1xHeight, Image1xWidth,
                             bottom ? 8 : 0, image);
  }

  Images images_;
  Noritake_VFD_GU7000& screen_;
};

class ScollManager {
 public:
  ScollManager(Image2x& hundreds, Image2x& tens, Image2x& ones)
      : hundreds_scroller_(hundreds, blank_2x),
        tens_scroller_(tens, blank_2x),
        ones_scroller_(ones, blank_2x) {}

  void Calculate(int8_t theNew, int8_t old) {
    original_old_ = old;
    original_new_ = theNew;
    diff_direction_ =
      (theNew - old) > 0 ? ScrollDirection::Down : ScrollDirection::Up;
    current_number_ = abs(old);
    const uint8_t old_ones = current_number_ % 10;
    ones_lines_left_till_10_ = diff_direction_ == ScrollDirection::Down
                                 ? (10 - old_ones)
                                 : (old_ones - 0);
    current_diff_magnitude_ = abs(theNew - old);
  }

  bool Finished() const {
    return false;
  }

  // 28
  // 29 // Add ticks to 10s, and apply in ApplyOnce
  // 20

  void ApplyOnce() {
    const uint8_t tens = (current_number_ % 100) / 10;
    const uint8_t ones = (current_number_ % 10);

    bool ones_done = ones_scroller_.ScrollInDirection(diff_direction_);

    if (tens_lines_left_ > 0) {
      tens_lines_left_ -= 1;
      tens_scroller_.ScrollInDirection(diff_direction_);
    }

    if (ones_done) {
      const uint8_t now_tens = (current_number_ % 100) / 10;
      current_number_ += diff_direction_ == ScrollDirection::Down ? 1 : -1;
      const uint8_t current_tens = (current_number_ % 100) / 10;

      ones_scroller_.SetNextImage(*number_2x_images[current_number_ % 10]);

      if (current_tens != now_tens) {
        tens_lines_left_ += ImageHeight2x;
      }
    }
  }

 private:
  int8_t original_old_{0};
  int8_t original_new_{0};
  uint8_t current_number_{0};
  uint8_t current_diff_magnitude_{0};
  uint8_t ones_lines_left_till_10_{0};
  uint8_t tens_lines_left_{0};
  ScrollDirection diff_direction_{ScrollDirection::Down};
  ScrollerT hundreds_scroller_;
  ScrollerT tens_scroller_;
  ScrollerT ones_scroller_;
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

  r.InitializeImages(-23, 2);

  r.DrawTemperature('F');
  r.DrawHumidity();
  r.DrawSetPoint(-32);
  r.DrawHeatingStatus(false);

  auto& images = r.GetImages();

  ScollManager m(images.temperature_hundreds_or_minus_,
                 images.temperature_tens_, images.temperature_ones_);
  m.Calculate(-15, -23);

  while (!m.Finished()) {
    m.ApplyOnce();
    r.DrawTemperature('F');
  }

  return 0;
}

// ==================================================== //
