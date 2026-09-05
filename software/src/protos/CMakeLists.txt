# Protos static library for ESP-IDF (comms build).
# Mirrors sources from meson.build; bitproto is in repo submodules.
idf_component_register(
  SRCS
    "HaCommandMailBox.cpp"
    "HomeAssistantSerial_bp.c"
    "ThermoSaveData_bp.c"
    "checksum.cpp"
    "../../submodules/bitproto/lib/c/bitproto.c"
  INCLUDE_DIRS
    "."
    "../../submodules/bitproto/lib/c"
)
