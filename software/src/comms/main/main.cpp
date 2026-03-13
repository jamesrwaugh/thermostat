/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <ThermoSaveData_bp.h>
#include <driver/uart.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <hal/uart_types.h>
#include <nvs_flash.h>

#include <cstdint>

#include "states/machine.hpp"

void wifi_init(void) {
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

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI("init", "wifi_init_sta finished.");
}

void serial_init(QueueHandle_t& uart_queue) {
  const uart_port_t uart_num = UART_NUM_0;

  uart_config_t uart_config = {
      .baud_rate = 9600,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
      .source_clk = uart_sclk_t::UART_SCLK_DEFAULT,
      .flags = {.allow_pd = 0, .backup_before_sleep = 0},
  };

  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

  ESP_ERROR_CHECK(uart_set_pin(uart_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

  const int uart_buffer_size = 256;
  ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size,
                                      uart_buffer_size, 10, &uart_queue, 0));

  uint8_t data[uart_buffer_size];

  while (1) {
    // Read data from the UART
    int len = uart_read_bytes(UART_NUM_1, data, uart_buffer_size,
                              20 / portTICK_PERIOD_MS);

    // Write data back to the UART
    uart_write_bytes(UART_NUM_1, (const char*)data, len);
  }
}

extern "C" void app_main(void) {
  QueueHandle_t uart_queue;

  wifi_init();
  serial_init(uart_queue);

  Machine m;
  m.SwitchState(State::Type::WifiScan);

  while (true) {
    vTaskDelay(1000 /
               portTICK_PERIOD_MS);  // DOES THE SOFTAP CONTINUE TO EXIST IN A
                                     // FTM FRIENDLY STATE? IS THE WIFI PAUSED?
  }
}