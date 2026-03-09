#include "state.hpp"

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
                          void* event_data) {};

}  // namespace State