#include "machine.hpp"

#include <etl/placement_new.h>

#include "state.hpp"
#include "wifi_connect_state.hpp"
#include "wifi_scan_state.hpp"

void Machine::SwitchState(State::Type s) {
  auto address = CurrentState.get_address<State::Base>();

  switch (s) {
    case State::Type::WifiScan:
      ::new (address) WifiScanState(*this);
      break;
    case State::Type::WifiConnect:
      ::new (address) WifiConnectState(*this, "ssid", "password");
      break;
    case State::Type::WifiConnectFailed:
    case State::Type::WifiConnectted:
    case State::Type::MqttConnect:
    case State::Type::MqttConnected:
      break;
  }
}
