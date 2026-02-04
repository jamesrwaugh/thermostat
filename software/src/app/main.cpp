#include <HomeAssistantSerial_bp.h>
#include <Noritake_VFD_GU7000.h>
#include <avr/interrupt.h>
#include <driver_ds1307.h>

#include <checksum.hpp>
#include <driver_rs_wrapper.hpp>

#include "event.hpp"
#include "states/machine.hpp"
#include "states/program_screen.hpp"

Machine machine;
volatile bool g10MillisecondPassed = false;

void OnButtonPressed(Button b) {
  switch (b) {
    case Button::Up:
      machine.receive(Event::UpButtonPressed());
      break;
    case Button::Down:
      machine.receive(Event::DownButtonPressed());
      break;
    case Button::Select:
      machine.receive(Event::SelectButtonPressed());
      break;
    case Button::Fan:
      machine.receive(Event::FanButtonPressed());
      break;
    case Button::Heat:
      machine.receive(Event::HeatButtonPressed());
      break;
    case Button::ReverseValveOnHeat:
      machine.receive(
          Event::ReverseValveModeChanged(ReverseValveModeT::OnForHeating));
      break;
    case Button::ReverseValveOnCool:
      machine.receive(
          Event::ReverseValveModeChanged(ReverseValveModeT::OnForCooling));
      break;
  }
}

class HaCommandMailBox {
 public:
  HaCommandMailBox(Machine& m) : m_{m} {}

  static_assert(BYTES_LENGTH_HA_COMMAND > 1,
                "HaCommand must be greater than 1");

  void ReceiveByte(uint8_t byte) {
    if (current_bytes_ < sizeof(buffer_)) {
      buffer_[current_bytes_++] = byte;
    } else {
      HaCommand c;
      DecodeHaCommand(&c, buffer_);
      uint8_t checksum_now = checksum(buffer_ + 1, sizeof(buffer_) - 1);
      if (checksum_now == c.checksum) {
        m_.receive(c);
      }
      current_bytes_ = 0;
    }
  }

 private:
  Machine& m_;
  uint8_t current_bytes_{0};
  uint8_t buffer_[BYTES_LENGTH_HA_COMMAND];
};

int main() {
  DriverInit();
  machine.start();

  uint8_t lastHalfSecondCount = 0;
  uint8_t lastSecondCount = 0;

  HaCommandMailBox mail(machine);

  ProgramScreenState::Screen_ = &DriverGetScreenHandle();
  ScreenBox::Screen_ = &DriverGetScreenHandle();

  while (true) {
    if (g10MillisecondPassed) {
      auto button = DriverReadButton();
      if (button != -1) {
        OnButtonPressed(static_cast<Button>(button));
      }
      g10MillisecondPassed = false;
      lastHalfSecondCount += 1;
      lastSecondCount += 1;
    }

    if (lastHalfSecondCount >= 50) {
      lastHalfSecondCount = 0;
      machine.receive(Event::HalfSecondPassed());
    }

    if (lastSecondCount >= 100) {
      lastSecondCount = 0;
      machine.receive(Event::SecondPassed());
    }

    uint8_t byte;
    if (DriverGetSerialByte(&byte)) {
      mail.ReceiveByte(byte);
    }

    DriverMcuSleep();
  }

  return 0;
}

ISR(TIMER1_COMPA_vect) {
  g10MillisecondPassed = true;
}

void operator delete(void*, unsigned int) {}
