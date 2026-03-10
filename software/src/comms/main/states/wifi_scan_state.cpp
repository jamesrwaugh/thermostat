#include "wifi_scan_state.hpp"

#include <esp_wifi.h>
#include <stdint.h>
#include <string.h>

#include <HomeAssistantSerial.hpp>

WifiScanState::WifiScanState(Machine& m) : State::Base(State::Type::WifiScan) {
  RegisterEspEvent(WIFI_EVENT, WIFI_EVENT_SCAN_DONE);
  ESP_ERROR_CHECK(esp_wifi_scan_start(nullptr, false));
}

WifiScanState::~WifiScanState() {
  esp_wifi_clear_ap_list();
}

void WifiScanState::HandleEspEvent(esp_event_base_t event_base,
                                   int32_t event_id,
                                   void* event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
    scan_results_count_ = MAX_SCAN_RESULTS;
    ESP_ERROR_CHECK(
        esp_wifi_scan_get_ap_records(&scan_results_count_, scan_results_));

    auto* ev = static_cast<wifi_event_sta_scan_done_t*>(event_data);
    WriteSerialResponse(SerialInTopicKey::WifiScanDone, ev->status,
                        scan_results_count_);
  }
}

void WifiScanState::HandleSerialEvent(const SerialCommand& c) {
  switch (static_cast<SerialOutTopicKey>(c.topic_key)) {
    case SerialOutTopicKey::WifiRequestSsid:
      const uint8_t which = c.payload[0];
      const auto name = scan_results_[which].ssid;
      WriteSerialResponse(SerialInTopicKey::WifiSentSsidName, name,
                          strlen((char*)name));
      break;
  }
}
