

#include <libft4222.h>

#include <cstdint>
#include <vector>

extern FT_HANDLE gHandle;

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
