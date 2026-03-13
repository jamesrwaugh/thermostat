#include <etl/alignment.h>
#include <etl/largest.h>
#include <etl/string.h>

#include "idle.hpp"
#include "state.hpp"
#include "wifi_connect_state.hpp"
#include "wifi_scan_state.hpp"

struct WifiConfig {
  etl::string<32> ssid;
  etl::string<64> password;
};

class Machine {
 public:
  Machine();
  void SwitchState(State::Type s);
  WifiConfig& GetWifiConfig();

 private:
  static constexpr size_t StatesMaxSize =
      etl::largest<Idle, WifiConnectState, WifiScanState>::size;

  static constexpr size_t StatesAlignment =
      etl::largest<Idle, WifiConnectState, WifiScanState>::alignment;

  etl::aligned_storage<StatesMaxSize, StatesAlignment>::type CurrentState;

  WifiConfig wifi_config_;
};