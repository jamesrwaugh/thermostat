#pragma once

#include <stdint.h>

#include "HomeAssistantSerial_bp.h"
#include "esp_wifi_types_generic.h"
#include "state.hpp"

class Machine;

class WifiScanState final : public State::Base {
 public:
  WifiScanState(Machine& m);
  ~WifiScanState();

 protected:
  virtual void HandleEspEvent(esp_event_base_t event_base,
                              int32_t event_id,
                              void* event_data) override;
  void HandleSerialEvent(const SerialCommand& c) override;

 private:
  static constexpr uint8_t MAX_SCAN_RESULTS = 4;
  uint16_t scan_results_count_{0};
  wifi_ap_record_t scan_results_[MAX_SCAN_RESULTS];
};