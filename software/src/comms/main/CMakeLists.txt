idf_component_register(SRCS "main.cpp" "states/wifi_connect_state.cpp"
                       PRIV_REQUIRES esp_wifi nvs_flash protos
                       INCLUDE_DIRS "")