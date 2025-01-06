#include "load.hpp"

#include <simavr/avr_uart.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_gdb.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <simavr-toolbox/sim_base.hpp>
#include <string>
#include <string_view>

void sim_debug_log(const char *fmt, va_list args) {
  va_list args2;
  va_copy(args2, args);

  int len = 1 + vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  std::string message(len, 0);
  vsnprintf(message.data(), message.length(), fmt, args2);
  va_end(args2);
}

avr_t *LoadFirmware(std::string_view filename, bool gdb) {
  avr_t *avr = nullptr;

  elf_firmware_t f;
  memset(&f, 0, sizeof(f));

  sim_debug_log("Firmware pathname is %s\n", filename.data());
  elf_read_firmware(filename.data(), &f);

  sim_debug_log("firmware %s f=%d mmcu=%s\n", filename.data(), (int)f.frequency,
                f.mmcu);

  avr = avr_make_mcu_by_name(f.mmcu);

  if (!avr) {
    sim_debug_log("AVR '%s' not known\n", f.mmcu);
    std::abort();
  }

  avr_init(avr);

  avr_load_firmware(avr, &f);

  // disable the stdio dump, as we have a TUI output
  uint32_t flags = 0;
  avr_ioctl(avr, AVR_IOCTL_UART_GET_FLAGS('0'), &flags);
  flags &= ~AVR_UART_FLAG_STDIO;
  avr_ioctl(avr, AVR_IOCTL_UART_SET_FLAGS('0'), &flags);

  if (gdb) {
    avr->gdb_port = 1234;
    avr->state = cpu_Stopped;
    avr_gdb_init(avr);
  }

  return avr;
}