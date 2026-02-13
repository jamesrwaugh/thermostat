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

#include <algorithm>
#include <cstring>
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

/* 5x8 image "fire0" */
static const Image1x image_fire0 = {0x4e, 0x3f, 0x7f, 0x1f, 0x0e};
/* 5x8 image "fire1" */
static const Image1x image_fire1 = {0x8e, 0x3f, 0x7f, 0x3f, 0x0e};
/* 5x8 image "fire2" */
static const Image1x image_fire2 = {0x0e, 0x3f, 0x7f, 0xff, 0x06};
/* 5x8 image "fire3" */
static const Image1x image_fire3 = {0x0e, 0xbf, 0xff, 0x7f, 0x26};
/* 5x8 image "fire4" */
static const Image1x image_fire4 = {0x0e, 0xff, 0x3f, 0x1f, 0x4e};
/* 5x8 image "fire5" */
static const Image1x image_fire5 = {0x3e, 0x7f, 0xbf, 0x1f, 0x8e};
/* 5x8 image "fire6" */
static const Image1x image_fire6 = {0x3e, 0x1f, 0x7f, 0x1f, 0x06};
/* 5x8 image "fire7" */
static const Image1x image_fire7 = {0x06, 0x1f, 0xbf, 0x1f, 0x0e};
/* 5x8 image "fire8" */
static const Image1x image_fire8 = {0x2e, 0x1f, 0x7f, 0x3f, 0x0e};

static constexpr uint8_t FiresImagesCount = 9;

static const Image1x* fire_images[FiresImagesCount] = {
  &image_fire0, &image_fire1, &image_fire2, &image_fire3, &image_fire4,
  &image_fire5, &image_fire6, &image_fire7, &image_fire8,
};

class Renderer {
 public:
  static constexpr uint8_t CharacterWidth1x = 7;
  static constexpr uint8_t ScreenWidth = 112;
  static constexpr uint8_t TemperatureXPos = 0;
  static constexpr uint8_t HumidityXPos =
    TemperatureXPos + (3 * ImageWidth2x) + CharacterWidth1x + CharacterWidth1x;
  static constexpr uint8_t UnactiveHeatImageIdx = 3;

  static_assert(UnactiveHeatImageIdx < FiresImagesCount,
                "Default fire index out of bounds");

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
      screen_.print(imagePosition - (CharacterWidth1x * 2) - 2, 9, "ON");
      fire_state_ += 1;
      if (fire_state_ == FiresImagesCount) {
        fire_state_ = 0;
      }
      Draw1xImage(imagePosition, true, *fire_images[fire_state_]);
    } else {
      Draw1xImage(imagePosition, true, *fire_images[UnactiveHeatImageIdx]);
    }
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

    screen_.GU7000_drawImage(tensSpot, 1, ImageWidth2x, ImageHeight2x, tens);

    screen_.GU7000_drawImage(onesSpot, 1, ImageWidth2x, ImageHeight2x, ones);

    screen_.print(suffixSpot, 1, suffix);
  }

  void Draw1xImage(uint8_t xPositionDots, bool bottom, const Image1x& image) {
    screen_.GU7000_drawImage(xPositionDots, Image1xHeight + 1, Image1xWidth,
                             bottom ? 8 : 0, image);
  }

  uint8_t fire_state_{0};
  Images images_;
  Noritake_VFD_GU7000& screen_;
};

class ScollManager {
 public:
  ScollManager(Image2x& hundreds, Image2x& tens, Image2x& ones)
      : hundreds_scroller_(hundreds, blank_2x),
        tens_scroller_(tens, blank_2x),
        ones_scroller_(ones, blank_2x) {}

  void Calculate(int8_t old, int8_t theNew) {
    diff_direction_ =
      (theNew < old) ? ScrollDirection::Up : ScrollDirection::Down;
    current_number_ = old;
    goal_number_ = theNew;
    RecalculateImages();
  }

  bool Finished() const {
    return current_number_ == goal_number_;
  }

  void ApplyOnce() {
    bool ones_done = ones_scroller_.ScrollInDirection(diff_direction_);

    if (hundreds_lines_left_ > 0) {
      hundreds_lines_left_ -= 1;
      hundreds_scroller_.ScrollInDirection(diff_direction_);
    }

    if (tens_lines_left_ > 0) {
      tens_lines_left_ -= 1;
      tens_scroller_.ScrollInDirection(diff_direction_);
    }

    if (ones_done) {
      current_number_ += Delta();
      RecalculateImages();
    }
  }

  void RecalculateImages() {
    ones_scroller_.SetNextImage(
      *number_2x_images[Ones(current_number_ + Delta())]);
    RecalculateHundredsLines();
    RecalculateTensLines();
  }

  void RecalculateHundredsLines() {
    const uint8_t curentHundreds = Hundreds(current_number_);
    const int8_t nextNumber = current_number_ + Delta();
    const uint8_t nextHundreds = Hundreds(nextNumber);
    if (curentHundreds != nextHundreds) {
      hundreds_lines_left_ += ImageHeight2x;
      hundreds_scroller_.SetNextImage(*number_2x_images[nextHundreds]);
    } else if (current_number_ < 0 && nextNumber >= 0) {
      hundreds_lines_left_ += ImageHeight2x;
      hundreds_scroller_.SetNextImage(blank_2x);
    } else if (current_number_ >= 0 && nextNumber < 0) {
      hundreds_lines_left_ += ImageHeight2x;
      hundreds_scroller_.SetNextImage(minus_2x);
    } else {
      hundreds_lines_left_ = 0;
    }
  }

  void RecalculateTensLines() {
    const uint8_t nextTens = Tens(current_number_ + Delta());
    if (Tens(current_number_) != nextTens) {
      tens_lines_left_ += ImageHeight2x;
      tens_scroller_.SetNextImage(*number_2x_images[nextTens]);
    } else {
      tens_lines_left_ = 0;
    }
  }

  static inline uint8_t Hundreds(int8_t number) {
    if (number < 0) number = -number;
    return number / 100;
  }

  static inline uint8_t Tens(int8_t number) {
    if (number < 0) number = -number;
    return (number % 100) / 10;
  }

  static inline uint8_t Ones(int8_t number) {
    if (number < 0) number = -number;
    return number % 10;
  }

  int8_t Delta() const {
    return diff_direction_ == ScrollDirection::Up ? -1 : 1;
  }

 private:
  int8_t goal_number_{0};
  int8_t current_number_{0};
  uint8_t tens_lines_left_{0};
  uint8_t hundreds_lines_left_{0};
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

  //                      old   new
  // 000 neg,neg,newless  -10 ? -11  1 up 1
  // 001 neg,neg,newmore  -10 ? -9   1 down -1
  // 010 neg,pos,newless  N/A
  // 011 neg,pos,newmore  -10 ?  10 20 down -1
  // 100 pos,neg,newless   10 ? -10 20 up 1
  // 101 pos,neg,newmore  N/A
  // 110 pos,pos,newless   10 ?   5  5 up 1
  // 111 pos,pos,newmore   10 ?  20  5 up -1

  m.Calculate(oldTemp, newTemp);

  // unsigned count = 0;
  // while (!m.Finished()) {
  //   m.ApplyOnce();
  //   r.DrawTemperature('F');
  //   usleep(std::clamp(50000u - (count * 500), 25000u, 50000u));
  //   count += 1;
  // }

  while (1) {
    r.DrawHeatingStatus(true);
    if (!m.Finished()) {
      m.ApplyOnce();
      r.DrawTemperature('F');
    }
    usleep(75000);
  }

  return 0;
}

// ==================================================== //
