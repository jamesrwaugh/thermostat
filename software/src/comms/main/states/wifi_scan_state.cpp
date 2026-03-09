#include "wifi_scan_state.hpp"

#include <esp_wifi.h>

WifiScanState::WifiScanState(Machine& m) : State::Base(State::Type::WifiScan) {
  esp_wifi_scan_start(nullptr, false);
}
