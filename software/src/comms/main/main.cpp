/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <ThermoSaveData_bp.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <string.h>

#include "config.hpp"

enum class State : uint8_t {
  WifiScan = 0,
  WifiConnect,
  WifiConnectFailed,
  WifiConnectted,
  MqttConnect,
  MqttConnected,
};

class Machine {
 public:
  void SwitchState(State s) {}

 private:
};

class WifiConnectState {
  static constexpr auto WifiEventType = &WIFI_EVENT;
  static constexpr auto WifiEventId = ESP_EVENT_ANY_ID;
  static constexpr auto IpEventType = &IP_EVENT;
  static constexpr auto IpEventId = IP_EVENT_STA_GOT_IP;

 public:
  WifiConnectState(Machine& m, const char* ssid, const char* password)
      : machine_{m} {
    auto handler_lambda = [](void* event_handler_arg,
                             esp_event_base_t event_base, int32_t event_id,
                             void* event_data) {
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
    wifi_config.sta.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
  }

  ~WifiConnectState() {
    esp_event_handler_instance_unregister(*WifiEventType, WifiEventId,
                                          instance_any_id);
    esp_event_handler_instance_unregister(*IpEventType, IpEventId,
                                          instance_got_ip);
  }

  void ConnectEventHandler(esp_event_base_t event_base,
                           int32_t event_id,
                           void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
      esp_wifi_connect();
    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {
      if (retry_num_ < EXAMPLE_ESP_MAXIMUM_RETRY) {
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

  static constexpr const char* wifi_tag_ = "wifi station";
  Machine& machine_;
  wifi_config_t wifi_config;
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  uint8_t retry_num_{0};
};

void wifi_init_sta(void) {
  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_LOGI("init", "wifi_init_sta finished.");
}

extern "C" void app_main(void) {
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  if (CONFIG_LOG_MAXIMUM_LEVEL > CONFIG_LOG_DEFAULT_LEVEL) {
    /* If you only want to open more logs in the wifi module, you need to make
     * the max level greater than the default level, and call
     * esp_log_level_set() before esp_wifi_init() to improve the log level of
     * the wifi module. */
    esp_log_level_set("init",
                      static_cast<esp_log_level_t>(CONFIG_LOG_MAXIMUM_LEVEL));
  }

  ESP_LOGI("init", "ESP_WIFI_MODE_STA");

  struct ThermoSaveData s;
  unsigned char b[BYTES_LENGTH_THERMO_SAVE_DATA];
  EncodeThermoSaveData(&s, b);

  wifi_init_sta();

  Machine m;
  WifiConnectState ss(m, EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);

  while (true) {
    vTaskDelay(1000 /
               portTICK_PERIOD_MS);  // DOES THE SOFTAP CONTINUE TO EXIST IN A
                                     // FTM FRIENDLY STATE? IS THE WIFI PAUSED?
  }
}