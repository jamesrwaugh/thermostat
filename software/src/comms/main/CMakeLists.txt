idf_component_register(
    SRCS 
    "main.cpp" 
    "states/wifi_connect_state.cpp" 
    "states/wifi_scan_state.cpp" 
    "states/state.cpp"
    "states/machine.cpp"
    PRIV_REQUIRES esp_wifi nvs_flash protos etl
    INCLUDE_DIRS ""
)