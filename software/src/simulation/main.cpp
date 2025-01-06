#include "sim_thermostat.hpp"

void sim_debug_log(const char *fmt, va_list args) {
  va_list args2;
  va_copy(args2, args);

  int len = 1 + vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  std::string message(len, 0);
  vsnprintf(message.data(), message.length(), fmt, args2);
  va_end(args2);
}

int main() {
  set_sim_debug_log(sim_debug_log);
  SimAvrThermostat Thermo("", false);
  return 0;
}