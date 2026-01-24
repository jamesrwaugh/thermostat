#include "sim_gu7000_i2c.hpp"

#include <lodepng.h>

#include <iostream>

SimGu7000I2C::SimGu7000I2C(avr_t* avr) : SimAvrI2CSmarterComponent(avr, 0x50) {}

const SimGu7000Real::DisplayMemory& SimGu7000I2C::GetDisplayMemory() const {
  return screen_.GetDisplayMemory();
}

void SimGu7000I2C::OnMillisecondPassed() {
  if (screen_dirty_) {
    last_command_debounce_ms_ += 1;
    if (last_command_debounce_ms_ >= 250) {
      WriteDisplayMemoryToPng();
      screen_dirty_ = false;
    }
  }
}

void SimGu7000I2C::OnDataReceived(const std::vector<uint8_t>& data) {
  last_command_debounce_ms_ = 0;
  screen_dirty_ = true;
  for (auto byte : data) {
    screen_.ProcessCommand(byte);
  }
}

void SimGu7000I2C::WriteDisplayMemoryToPng() {
  const auto& memory = screen_.GetDisplayMemory();
  unsigned width = screen_.Width();
  unsigned height = screen_.Height();

  std::vector<unsigned char> image;
  image.resize(width * height * 4);

  for (unsigned y = 0; y < height; y++)
    for (unsigned x = 0; x < width; x++) {
      image[4 * width * y + 4 * x + 0] = 0;
      image[4 * width * y + 4 * x + 1] = 0;
      image[4 * width * y + 4 * x + 2] = memory[x][y] ? 255 : 0;
      image[4 * width * y + 4 * x + 3] = 255;
    }

  unsigned error = lodepng::encode(
      "/home/james/Desktop/Git/squaredel/thermostat/software/screen.png", image,
      width, height);

  if (error)
    std::cout << "encoder error " << error << ": " << lodepng_error_text(error)
              << std::endl;
}
