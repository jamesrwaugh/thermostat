#pragma once

#include <stdint.h>

struct MqttState {
  static constexpr uint8_t gMqttTimeoutMaxSeconds = 60;

  void IncreaseTimeout() {
    if (MqttDisconnectedSeconds < gMqttTimeoutMaxSeconds) {
      MqttDisconnectedSeconds += 1;
    }
  }

  void ResetTimeout() {
    MqttDisconnectedSeconds = 0;
  }

  bool IsMqttConnected() const {
    return MqttDisconnectedSeconds < gMqttTimeoutMaxSeconds;
  }

 private:
  enum class SmartMode : uint8_t {
    Controller,
    HomeAssistant,
  };

  SmartMode Mode{SmartMode::Controller};
  uint8_t MqttDisconnectedSeconds{0};
};
