#include "machine.hpp"

#include <etl/placement_new.h>

#include "idle.hpp"
#include "state.hpp"
#include "wifi_connect_state.hpp"
#include "wifi_scan_state.hpp"

Machine::Machine() {
  ::new (CurrentState.get_address<State::Base>()) Idle();
}

void Machine::SwitchState(State::Type s) {
  auto address = CurrentState.get_address<State::Base>();

  address->~Base();

  switch (s) {
    case State::Type::Idle:
      ::new (address) Idle();
      break;
    case State::Type::WifiScan:
      ::new (address) WifiScanState(*this);
      break;
    case State::Type::WifiConnect:
      ::new (address) WifiConnectState(*this);
      break;
    case State::Type::WifiConnectFailed:
    case State::Type::WifiConnectted:
    case State::Type::MqttConnect:
    case State::Type::MqttConnected:
      break;
  }
}

WifiConfig& Machine::GetWifiConfig() {
  return wifi_config_;
}
