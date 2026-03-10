#include <esp_event.h>
#include <esp_wifi.h>

#include "state.hpp"

class Machine;

class WifiConnectState final : public State::Base {
 public:
  WifiConnectState(Machine& m, const char* ssid, const char* password);

 protected:
  virtual void HandleEspEvent(esp_event_base_t event_base,
                              int32_t event_id,
                              void* event_data) override;

 private:
  static constexpr const char* wifi_tag_ = "wifi station";
  static constexpr uint8_t max_retries_ = 5;
  Machine& machine_;
  wifi_config_t wifi_config;
  uint8_t retry_num_{0};
};
