#include <driver_rs_wrapper.hpp>

void OnButtonPressed(Button b, void *userData) {
  (void)b;
  (void)userData;
}

void OnSerialMessage(const char *message, uint16_t messageLen, void *userData) {
  (void)message;
  (void)messageLen;
  (void)userData;
}

void OnSecondPassed(void *userData) {
  (void)userData;
}

int main() {
  AvrDriverCallbacks callbacks{
      .OnButtonPressed = OnButtonPressed,
      .OnSerialMessage = OnSerialMessage,
      .OnSecondPassed = OnSecondPassed,
  };

  DriverInit(callbacks, nullptr);

  return 0;
}