#include "comm_event.hpp"

#include <driver_rs_wrapper.hpp>

namespace CommEvent {

struct SerialPrintVisitor {
  void operator()(TempChangedEvent& e) {
    uint8_t b[BYTES_LENGTH_TEMP_CHANGED_EVENT];
    EncodeTempChangedEvent(&e, b);
    DriverWriteSerialPort(b, sizeof(b));
  }

  void operator()(SetPointChangedEvent& e) {
    uint8_t b[BYTES_LENGTH_SET_POINT_CHANGED_EVENT];
    EncodeSetPointChangedEvent(&e, b);
    DriverWriteSerialPort(b, sizeof(b));
  }

  void operator()(HeatingModeChangedEvent& e) {
    uint8_t b[BYTES_LENGTH_HEATING_MODE_CHANGED_EVENT];
    EncodeHeatingModeChangedEvent(&e, b);
    DriverWriteSerialPort(b, sizeof(b));
  }

  void operator()(SettingsChangedEvent& e) {
    uint8_t b[BYTES_LENGTH_SETTINGS_CHANGED_EVENT];
    EncodeSettingsChangedEvent(&e, b);
    DriverWriteSerialPort(b, sizeof(b));
  }
};

void PrintCommEventToSerial(CommEventType& ev) {
  SerialPrintVisitor v;
  ev.accept(v);
}

}  // namespace CommEvent