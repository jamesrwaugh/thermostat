#include <lodepng.h>
#include <sys/types.h>

#include <cstdint>
#include <iostream>
#include <sim_gu7000_real.hpp>
#include <vector>

#include "./GU7000//Noritake_VFD_GU7000.h"

SimGu7000Real gSimGu7k;

void initPort() {}

void writePort(const uint8_t data, const uint8_t busyPin) {
  gSimGu7k.ProcessCommand(data);
}

void hardReset() {}

void encodeOneStep(const char* filename, std::vector<unsigned char>& image,
                   unsigned width, unsigned height) {
  // Encode the image
  unsigned error = lodepng::encode(filename, image, width, height);

  // if there's an error, display it
  if (error)
    std::cout << "encoder error " << error << ": " << lodepng_error_text(error)
              << std::endl;
}

struct Thing {
  int A;
};

constexpr uint8_t CharDotWidth = 7;
constexpr uint8_t CharDotHeight = 7;
constexpr uint8_t ImageWidth = 7;

const uint8_t gArrowImageData[ImageWidth] = {
    // clang-format off
    0b0000'1000, 
    0b0001'1000, 
    0b0011'1110, 
    0b0001'1000, 
    0b0000'1000,
    0b0000'0000,
    0b0000'0000,
    // clang-format on
};

int main() {
  Noritake_VFD_GU7000 gu(1);

  gu.GU7000_init();
  gu.GU7000_setCursor(0, 0);
  gu.print("Hello world!");
  gu.GU7000_drawImage(0, CharDotHeight, ImageWidth, 8, gArrowImageData);

  const auto& memory = gSimGu7k.GetDisplayMemory();

  // generate some image
  unsigned width = SimGu7000Real::DISPLAY_WIDTH,
           height = SimGu7000Real::DISPLAY_HEIGHT;
  std::vector<unsigned char> image;
  image.resize(width * height * 4);
  for (unsigned y = 0; y < height; y++)
    for (unsigned x = 0; x < width; x++) {
      image[4 * width * y + 4 * x + 0] = 0;
      image[4 * width * y + 4 * x + 1] = 0;
      image[4 * width * y + 4 * x + 2] = memory[x][y] ? 255 : 0;
      image[4 * width * y + 4 * x + 3] = 255;
    }

  encodeOneStep("out.png", image, width, height);

  return 0;
}