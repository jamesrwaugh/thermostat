#pragma once

#include <stdint.h>

enum class State : uint8_t {
  WifiScan = 0,
  WifiConnect,
  WifiConnectFailed,
  WifiConnectted,
  MqttConnect,
  MqttConnected,
};