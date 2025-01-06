#pragma once
#pragma once

#include <simavr/sim_avr.h>

#include <ftxui/component/receiver.hpp>
#include <ftxui/component/task.hpp>

class FtxUiSimulatedAvr {
 public:
  FtxUiSimulatedAvr(std::string_view filename, bool gdb);
  void BlockingLoop(std::atomic_bool&,
                    ftxui::Receiver<ftxui::Closure>& receiver);
  static avr_t* LoadFirmware(std::string_view filename, bool gdb);

 protected:
  virtual void RunOnceExtra();
  avr_irq_t* get_pin_irq(avr_t* avr, char pin, uint8_t index);
  avr_t* Avr_{nullptr};
};