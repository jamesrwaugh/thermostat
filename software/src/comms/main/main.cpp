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

#include "states/machine.hpp"

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
  m.SwitchState(State::Type::WifiScan);

  while (true) {
    vTaskDelay(1000 /
               portTICK_PERIOD_MS);  // DOES THE SOFTAP CONTINUE TO EXIST IN A
                                     // FTM FRIENDLY STATE? IS THE WIFI PAUSED?
  }
}