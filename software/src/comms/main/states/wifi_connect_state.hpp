#include <esp_event.h>
#include <esp_wifi.h>

class Machine;

class WifiConnectState {
  static constexpr auto WifiEventType = &WIFI_EVENT;
  static constexpr auto WifiEventId = ESP_EVENT_ANY_ID;
  static constexpr auto IpEventType = &IP_EVENT;
  static constexpr auto IpEventId = IP_EVENT_STA_GOT_IP;

 public:
  WifiConnectState(Machine& m, const char* ssid, const char* password);
  ~WifiConnectState();

  void ConnectEventHandler(esp_event_base_t event_base,
                           int32_t event_id,
                           void* event_data);

  static constexpr const char* wifi_tag_ = "wifi station";
  static constexpr uint8_t max_retries_ = 5;
  Machine& machine_;
  wifi_config_t wifi_config;
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  uint8_t retry_num_{0};
};
