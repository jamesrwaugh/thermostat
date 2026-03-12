#include <etl/alignment.h>
#include <etl/largest.h>

#include "state.hpp"
#include "wifi_connect_state.hpp"
#include "wifi_scan_state.hpp"

class Machine {
 public:
  void SwitchState(State::Type s);

 private:
  static constexpr size_t StatesMaxSize =
      etl::largest<WifiConnectState, WifiScanState>::size;

  static constexpr size_t StatesAlignment =
      etl::largest<WifiConnectState, WifiScanState>::alignment;

  etl::aligned_storage<StatesMaxSize, StatesAlignment>::type CurrentState;
};