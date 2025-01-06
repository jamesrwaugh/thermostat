#include <simavr/sim_avr.h>

#include <string_view>

void sim_debug_log(const char *fmt, va_list args);

avr_t *LoadFirmware(std::string_view filename, bool gdb);