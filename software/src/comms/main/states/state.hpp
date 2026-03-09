#pragma once

#include <esp_event.h>
#include <esp_event_base.h>
#include <stdint.h>
#include <sys/types.h>

namespace State {

enum class Type : uint8_t {
  WifiScan = 0,
  WifiConnect,
  WifiConnectFailed,
  WifiConnectted,
  MqttConnect,
  MqttConnected,
};

class Base {
 public:
  Base(Type s);
  ~Base();
  State::Type StateId() const;

 protected:
  void RegisterEspEvent(esp_event_base_t event_base, int32_t event_id);
  virtual void HandleEspEvent(esp_event_base_t event_base,
                              int32_t event_id,
                              void* event_data);

 private:
  struct EventRegisterInfo {
    esp_event_base_t event;
    int32_t event_id;
    esp_event_handler_instance_t handler;
  };

  static constexpr uint8_t MAX_EVENTS = 5;
  uint8_t event_count_{0};
  EventRegisterInfo registered_events_[MAX_EVENTS];
  const Type state_id_;
};

}  // namespace State
