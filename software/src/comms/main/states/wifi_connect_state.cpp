#include "wifi_connect_state.hpp"

#include <esp_log.h>
#include <string.h>

#include "machine.hpp"

WifiConnectState::WifiConnectState(Machine& m,
                                   const char* ssid,
                                   const char* password)
    : machine_{m} {
  auto handler_lambda = [](void* event_handler_arg, esp_event_base_t event_base,
                           int32_t event_id, void* event_data) {
    WifiConnectState* that = (WifiConnectState*)event_handler_arg;
    that->ConnectEventHandler(event_base, event_id, event_data);
  };

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      *WifiEventType, WifiEventId, handler_lambda, this, &instance_any_id));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      *IpEventType, IpEventId, handler_lambda, this, &instance_got_ip));

  memset(&wifi_config, 0, sizeof(wifi_config));

  /* Authmode threshold resets to WPA2 as default if password
   * matches WPA2 standards (password len => 8). If you want to
   * connect the device to deprecated WEP/WPA networks, Please set
   * the threshold value to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set
   * the password with length and format matching to
   * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
   */

  strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
  strncpy((char*)wifi_config.sta.password, password,
          sizeof(wifi_config.sta.password));
  wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

WifiConnectState::~WifiConnectState() {
  esp_event_handler_instance_unregister(*WifiEventType, WifiEventId,
                                        instance_any_id);
  esp_event_handler_instance_unregister(*IpEventType, IpEventId,
                                        instance_got_ip);
}

void WifiConnectState::ConnectEventHandler(esp_event_base_t event_base,
                                           int32_t event_id,
                                           void* event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (retry_num_ < max_retries_) {
      esp_wifi_connect();
      retry_num_++;
      ESP_LOGI(wifi_tag_, "retry to connect to the AP");
    } else {
      machine_.SwitchState(State::WifiConnectFailed);
    }
    ESP_LOGI(wifi_tag_, "connect to the AP fail");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
    ESP_LOGI(wifi_tag_, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    retry_num_ = 0;
    machine_.SwitchState(State::WifiConnectted);
  }
}
