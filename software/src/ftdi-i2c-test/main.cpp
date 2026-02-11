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
    current_message_.reserve(32);
  }

  ~AutoTwi() {
    uint16_t written = 0;
    FT4222_I2CMaster_Write(handle_, 0x50, current_message_.data(),
                           current_message_.size(), &written);
    current_message_.clear();
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

constexpr uint8_t ImageWidth2x = 12;
constexpr uint8_t ImageWidth2xHalfSize = ImageWidth2x;
constexpr uint8_t ImageWidth2xFullSize = ImageWidth2x * 2;
constexpr uint8_t ImageHeight2x = 16;

typedef uint8_t Image2xHalf[ImageWidth2xHalfSize];
typedef uint8_t Image2x[ImageWidth2xFullSize];

static const uint8_t image_0_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x3f, 0xf0, 0xc0, 0xcc, 0xc0, 0xcc,
  0xc3, 0x0c, 0xc3, 0x0c, 0xcc, 0x0c, 0xcc, 0x0c, 0x3f, 0xf0, 0x3f, 0xf0,
};

static const uint8_t image_1_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x0c, 0x30, 0x0c,
  0xff, 0xfc, 0xff, 0xfc, 0x00, 0x0c, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t image_2_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x30, 0x0c, 0x30, 0x0c, 0xc0, 0x3c, 0xc0, 0x3c,
  0xc0, 0xcc, 0xc0, 0xcc, 0xc3, 0x0c, 0xc3, 0x0c, 0x3c, 0x0c, 0x3c, 0x0c};

static const uint8_t image_3_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0xc0, 0x30, 0xc0, 0x30, 0xc0, 0x0c, 0xc0, 0x0c,
  0xcc, 0x0c, 0xcc, 0x0c, 0xf3, 0x0c, 0xf3, 0x0c, 0xc0, 0xf0, 0xc0, 0xf0,
};

static const uint8_t image_4_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x03, 0xc0, 0x0c, 0xc0, 0x0c, 0xc0,
  0x30, 0xc0, 0x30, 0xc0, 0xff, 0xfc, 0xff, 0xfc, 0x00, 0xc0, 0x00, 0xc0,
};

static const uint8_t image_5_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0xfc, 0x30, 0xfc, 0x30, 0xcc, 0x0c, 0xcc, 0x0c,
  0xcc, 0x0c, 0xcc, 0x0c, 0xcc, 0x0c, 0xcc, 0x0c, 0xc3, 0xf0, 0xc3, 0xf0,
};

static const uint8_t image_6_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x0f, 0xf0, 0x33, 0x0c, 0x33, 0x0c,
  0xc3, 0x0c, 0xc3, 0x0c, 0xc3, 0x0c, 0xc3, 0x0c, 0x00, 0xf0, 0x00, 0xf0,
};

static const uint8_t image_7_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xc0, 0x00, 0xc0, 0xfc, 0xc0, 0xfc,
  0xc3, 0x00, 0xc3, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xf0, 0x00, 0xf0, 0x00,
};

static const uint8_t image_8_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x3c, 0xf0, 0x3c, 0xf0, 0xc3, 0x0c, 0xc3, 0x0c,
  0xc3, 0x0c, 0xc3, 0x0c, 0xc3, 0x0c, 0xc3, 0x0c, 0x3c, 0xf0, 0x3c, 0xf0,
};

static const uint8_t image_9_2x[ImageWidth2xFullSize] = {
  0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x3c, 0x00, 0xc3, 0x0c, 0xc3, 0x0c,
  0xc3, 0x0c, 0xc3, 0x0c, 0xc3, 0x30, 0xc3, 0x30, 0x3f, 0xc0, 0x3f, 0xc0,
};

static const Image2x* const number_2x_images[10] = {
  &image_0_2x, &image_1_2x, &image_2_2x, &image_3_2x, &image_4_2x,
  &image_5_2x, &image_6_2x, &image_7_2x, &image_8_2x, &image_9_2x,
};

typedef const Image2x* const Image2x0Thru9[10];

class Scroller {
 public:
  static_assert(ImageWidth2xFullSize % 2 == 0,
                "Image width must be multiple of 2");

  Scroller(Noritake_VFD_GU7000& screen,
           uint8_t xPositionDots,
           uint8_t startingNumber,
           const Image2x0Thru9& images)
      : screen_{screen},
        images_{images},
        x_position_dots_{xPositionDots},
        current_number_{startingNumber} {
    memcpy(&image_, images[startingNumber], sizeof(Image2x));
  }

  void ScollUpOneLine() {
    for (uint8_t col = 0; col < ImageWidth2xFullSize; col += 2) {
      ScollUpColumn(col);
    }

    scrolled_lines_ += 1;

    if (scrolled_lines_ == ImageHeight2x) {
      scrolled_lines_ = 0;
      current_number_ = PrevNumber();
    }
  }

  void ScrollDownOneLine() {
    for (uint8_t col = 0; col < ImageWidth2xFullSize; col += 2) {
      ScrollDownColumn(col);
    }

    scrolled_lines_ -= 1;

    if (scrolled_lines_ == -static_cast<int8_t>(ImageHeight2x)) {
      scrolled_lines_ = 0;
      current_number_ = NextNumber();
    }
  }

  int8_t ScrolledCount() const {
    return scrolled_lines_;
  }

  uint8_t CurrentNumber() const {
    return current_number_;
  }

  void Draw() {
    screen_.GU7000_drawImage(x_position_dots_, 0, ImageWidth2x, ImageHeight2x,
                             image_);
  }

 private:
  void ScrollDownColumn(uint8_t col) {
    uint8_t& top = image_[col];
    uint8_t& bottom = image_[col + 1];

    uint8_t nextNumber = HaveScrolledUp() ? current_number_ : NextNumber();

    const auto& nextImage = *images_[nextNumber];

    uint8_t row = HaveScrolledUp() ? (ImageHeight2x - scrolled_lines_)
                                   : abs(scrolled_lines_);

    uint8_t nextCol = nextImage[col + (row < 8 ? 1 : 0)];

    bottom >>= 1;
    bottom |= (top & 1) ? (1 << 7) : 0;
    top >>= 1;
    top |= (nextCol & (1 << (row % 8))) ? (1 << 7) : 0;
  }

  void ScollUpColumn(uint8_t col) {
    uint8_t& top = image_[col];
    uint8_t& bottom = image_[col + 1];

    uint8_t prevNumber = HaveScrolledDown() ? current_number_ : PrevNumber();

    const auto& prevImage = *images_[prevNumber];

    uint8_t row = HaveScrolledDown() ? ImageHeight2x - abs(scrolled_lines_)
                                     : scrolled_lines_;

    uint8_t prevCol = prevImage[col + (row < 8 ? 0 : 1)];

    top <<= 1;
    top |= (bottom & (1 << 7)) ? 1 : 0;
    bottom <<= 1;
    bottom |= (prevCol & (1 << (7 - (row % 8)))) ? 1 : 0;
  }

  uint8_t NextNumber() const {
    return current_number_ < 9 ? current_number_ + 1 : 0;
  }

  uint8_t PrevNumber() const {
    return current_number_ == 0 ? 9 : current_number_ - 1;
  }

  bool HaveScrolledUp() const {
    return scrolled_lines_ > 0;
  }

  bool HaveScrolledDown() const {
    return scrolled_lines_ < 0;
  }

  Noritake_VFD_GU7000& screen_;
  Image2x image_;
  const Image2x0Thru9& images_;
  const uint8_t x_position_dots_{0};
  uint8_t current_number_{0};
  int8_t scrolled_lines_{0};  // >0 scrolled up, <0 scrolled down
};

void PrintNumberScrolled(FT_HANDLE handle,
                         Noritake_VFD_GU7000& screen,
                         Scroller& s) {
  AutoTwi t(handle);
  screen.print(50, 0, "   ");
  screen.print(50, 0, s.ScrolledCount(), 10);
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
    screen.GU7000_setScreenBrightness(20);
  }

  // Scroller s(screen, handle, image_7_2x, image_8_2x);

  // {
  //   AutoTwi t(handle);
  //   s.Draw();
  //   usleep(250000);
  // }

  // for (int i = 0; i < ImageHeight2x; ++i) {
  //   s.ScrollOneUp();
  //   s.Draw();
  //   usleep(20000);
  // }

  Scroller s(screen, 20, 5, number_2x_images);

  {
    AutoTwi t(handle);
    screen.print(50, 0, "  ");
    screen.print(50, 0, s.ScrolledCount(), 10);
  }

  {
    AutoTwi t(handle);
    s.Draw();
  }

  for (int i = 0; i < (ImageHeight2x * 2) + 8; ++i) {
    AutoTwi t(handle);
    s.ScrollDownOneLine();
    s.Draw();
    PrintNumberScrolled(handle, screen, s);
    usleep(20000 + 1300 * i);
  }

  for (int i = 0; i < 12; ++i) {
    AutoTwi t(handle);
    s.ScrollDownOneLine();
    s.Draw();
    PrintNumberScrolled(handle, screen, s);
    usleep(std::min(2000, 30000 - i * 500));
  }

  usleep(500000);

  for (int i = 0; i < ImageHeight2x; ++i) {
    AutoTwi t(handle);
    s.ScollUpOneLine();
    s.Draw();
    PrintNumberScrolled(handle, screen, s);
    usleep(20000 + 1300 * i);
  }

  usleep(500000);

  for (int i = 0; i < (ImageHeight2x * 3) + 2; ++i) {
    AutoTwi t(handle);
    s.ScrollDownOneLine();
    s.Draw();
    PrintNumberScrolled(handle, screen, s);
    usleep(std::min(40000 - 400 * i, 20000));
  }

  for (int i = 0; i < (ImageHeight2x * 5); ++i) {
    AutoTwi t(handle);
    s.ScollUpOneLine();
    s.Draw();
    PrintNumberScrolled(handle, screen, s);
    usleep(std::min(30000 - 400 * i, 20000));
  }

  return 0;
}

// ==================================================== //
