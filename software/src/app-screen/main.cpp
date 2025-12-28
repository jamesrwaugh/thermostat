#include <Noritake_VFD_GU7000.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <etl/alignment.h>
#include <etl/largest.h>
#include <etl/placement_new.h>
#include <util/delay.h>

#include <driver_rs_wrapper.hpp>

#include "Serial/HardwareSerial.h"
#include "ThermoSaveData_bp.h"
#include "screen.hpp"
#include "utils.hpp"

uint8_t AutoTwi::instanceCount_ = 0;

void setup() {
  // initialize twi prescaler and bit rate (250k)
  // Set prescaler value of 1
  TWSR &= ~_BV(TWPS0);
  TWSR &= ~_BV(TWPS1);
  TWBR = ((F_CPU / 250000) - 16) / 2;

  // Screen Busy
  // -> Input
  DDRB &= ~_BV(DDB1);
  PORTB &= ~_BV(PORTB1);

  // LED
  DDRB |= _BV(DDB0);
  PORTB &= ~_BV(PB0);

  // Buttons
  // -> Input pullup
  DDRA &= ~_BV(DDA0);
  DDRA &= ~_BV(DDA1);
  DDRA &= ~_BV(DDA2);
  DDRA &= ~_BV(DDD3);
  PORTA |= _BV(PORTA0) | _BV(PORTA1) | _BV(PORTA2) | _BV(PORTA3);
}

volatile bool g10MillisecondPassed = false;

void SetupInputTimer() {
  // prescaler clk / 1024
  TCCR1B |= _BV(CS12);
  TCCR1B &= ~_BV(CS11);
  TCCR1B |= _BV(CS10);

  // CTC mode on OCR1A, to stop at that value
  // WGM1[3:0] = 0100 = 4
  TCCR1B &= ~_BV(WGM13);
  TCCR1B |= _BV(WGM12);
  TCCR1A &= ~_BV(WGM11);
  TCCR1A &= ~_BV(WGM10);

  // Set OCR1A, approx ~10ms per overflow
  constexpr uint16_t top = 72;
  OCR1A = top;

  // Interrupt on compare A match
  TIMSK1 |= _BV(OCIE1A);
}

class ProgramMachine {
 public:
  ProgramMachine(ThermoSaveData& saveData) : SaveData_(saveData) {}

  void CheckInput() {
    auto newScreen = ScreenHandleInput();

    if (newScreen != ScreenT::NO_CHANGE) {
      SwitchScreen(newScreen);
    }
  }

  void OnHalfSecondPassed() {
    Screen.get_reference<ScreenC>().OnHalfSecondPassed();
  }

 private:
  ScreenT ScreenHandleInput() {
    uint8_t pina = PINA;
    auto& screen = Screen.get_reference<ScreenC>();

    if (UpButton.Add(pina & _BV(PINA0))) {
      return screen.OnUpPressed();
    } else if (DownButton.Add(pina & _BV(PINA1))) {
      return screen.OnDownPressed();
    } else if (SelectButton.Add(pina & _BV(PINA2))) {
      screen.OnSelectPressed();
    }

    return ScreenT::NO_CHANGE;
  }

  void SwitchScreen(ScreenT newScreen) {
    ScreenC* address = Screen.get_address<ScreenC>();

    address->~ScreenC();

    switch (newScreen) {
      case ScreenT::TempDisplayUnit:
        ::new (address) TempScreen(SaveData_, Boxes);
        break;
      case ScreenT::DateSet:
        ::new (address) DateScreen(SaveData_, Boxes);
        break;
      case ScreenT::TimeSet:
        ::new (address) TimeScreen(SaveData_, Boxes);
        break;
      case ScreenT::NO_CHANGE:
        // Should never happen
        ::new (address) TempScreen(SaveData_, Boxes);
        break;
    }

    address->InitDisplay();
  }

  static constexpr size_t ScreensMaxSize =
      etl::largest<TempScreen, DateScreen, TimeScreen>::size;

  static constexpr size_t ScreensAlignment =
      etl::largest<TempScreen, DateScreen, TimeScreen>::alignment;

  typedef etl::aligned_storage<ScreensMaxSize, ScreensAlignment>::type
      ScreensStorage;

  ThermoSaveData& SaveData_;
  ScreensStorage Screen;
  ScreenBoxStorage Boxes[10];
  DebounceState UpButton;
  DebounceState DownButton;
  DebounceState SelectButton;
  DebounceState TimeButton;
};

void mainLoop() {
  ThermoSaveData data;
  data.magic = THERMO_STATE_DATA_MAGIC;
  data.set_point = 39;
  data.temp_display_unit = TEMP_UNIT_FREEDOM;
  data.date.year = 25;
  data.date.month = 9;
  data.date.day = 16;
  data.time.hour = 6;
  data.time.minute = 37;
  data.time.second = 23;
  data.time.am_pm = TIME_PM;

  ProgramMachine machine(data);

  bool ledOn = false;
  uint8_t ledCount = 0;
  uint8_t halfSecondCount = 0;

  while (1) {
    if (g10MillisecondPassed) {
      g10MillisecondPassed = false;

      ledCount += 1;
      if (ledCount > 100) {
        ledCount = 0;
        ledOn = !ledOn;
        if (ledOn) {
          PORTB |= _BV(PB0);
        } else {
          PORTB &= ~_BV(PB0);
        }
      }

      machine.CheckInput();

      halfSecondCount += 1;
      if (halfSecondCount > 50) {
        halfSecondCount = 0;
        machine.OnHalfSecondPassed();
      }
    }
  }
}

int main() {
  setup();
  SetupInputTimer();
  sei();

  Noritake_VFD_GU7000 gu7k(1);

  ScreenBox::Screen_ = &gu7k;
  ScreenC::Screen_ = &gu7k;

  {
    AutoTwi twi;
    gu7k.GU7000_init();
    gu7k.GU7000_clearScreen();
  }

  Serial.begin();

  mainLoop();

  return 0;
}

ISR(TIMER1_COMPA_vect) {
  g10MillisecondPassed = true;
}

void operator delete(void*, unsigned int) {}
