#include "state.hpp"

#include <stdint.h>
#include <string.h>

#include <HomeAssistantSerial.hpp>
#include <checksum.hpp>

namespace State {

Base::Base(Type s) : state_id_{s} {}

Base::~Base() {
  for (uint8_t i = 0; i < event_count_; ++i) {
    const auto& item = registered_events_[i];
    esp_event_handler_instance_unregister(item.event, item.event_id,
                                          item.handler);
  }
}

Type Base::StateId() const {
  return state_id_;
}

void Base::RegisterEspEvent(esp_event_base_t event_base, int32_t event_id) {
  auto handler_lambda = [](void* event_handler_arg, esp_event_base_t event_base,
                           int32_t event_id, void* event_data) {
    Base* that = (Base*)event_handler_arg;
    that->HandleEspEvent(event_base, event_id, event_data);
  };

  auto& next = registered_events_[event_count_++];
  next.event = event_base;
  next.event_id = event_id;

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      event_base, event_id, handler_lambda, this, &next.handler));
}

void Base::HandleEspEvent(esp_event_base_t event_base,
                          int32_t event_id,
                          void* event_data) {}

void Base::HandleSerialEvent(const SerialCommand& c) {}

void Base::WriteHaSerialResponse(HaInTopicKey topic,
                                 uint8_t byte_one,
                                 uint8_t byte_two) const {
  HaCommand c;
  c.checksum = 0;
  c.topic_key = static_cast<uint8_t>(topic);
  c.payload_byte_one = byte_one;
  c.payload_byte_two = byte_two;
  uint8_t b[BYTES_LENGTH_HA_COMMAND];  // 0 0 0 0
  memset(b, 0, sizeof(b));
  EncodeHaCommand(&c, b);
  b[0] = checksum(b + 1, sizeof(b) - 1);
  // DriverWriteSerialPortRaw(b, sizeof(b));
}

void Base::WriteSerialResponse(SerialInTopicKey topic,
                               uint8_t byte_one,
                               uint8_t byte_two) const {
  uint8_t buffer[2] = {byte_one, byte_two};
  WriteSerialResponse(topic, buffer, 2);
}

void Base::WriteSerialResponse(SerialInTopicKey topic,
                               const uint8_t* data,
                               uint8_t dataLen) const {
  SerialCommand c;
  c.checksum = 0;
  c.topic_key = static_cast<uint8_t>(topic);
  c.payload_len = dataLen;
  memcpy(c.payload, data, dataLen);
  uint8_t b[BYTES_LENGTH_SERIAL_COMMAND];  // 0 0 0 0
  memset(b, 0, sizeof(b));
  EncodeSerialCommand(&c, b);
  b[0] = checksum(b + 1, sizeof(b) - 1);
}

}  // namespace State