#include "driver.hpp"
#include "Serial/HardwareSerial.h"
#include "driver_rs_wrapper.hpp"
#include <etl/debounce.h>
#include <twi_master.h>

AvrDrivers::AvrDrivers(const AvrDriverCallbacks &callbacks, void *userData)
    : Screen(19), Serial_(Serial), Callbacks_(callbacks), UserData_{userData} {}

void AvrDrivers::Setup() {
  SetupPins();
  SetupI2C();
  SetupTimer();
  SetupRTC();
  SetupScreen();
}

void AvrDrivers::SetupI2C() {
  // set no pullups for SDA / SCL
  DDRC &= ~_BV(PC1);
  DDRC &= ~_BV(PC0);
  PORTC &= ~_BV(PC1);
  PORTC &= ~_BV(PC0);

  // initialize twi prescaler and bit rate (250k)
  // Set prescaler value of 1
  TWSR &= ~_BV(TWPS0);
  TWSR &= ~_BV(TWPS1);
  TWBR = ((F_CPU / 50000) - 16) / 2;
}

void AvrDrivers::SetupPins() {
  // Up button, down button, cooling, heating
  // -> Input pullup
  DDRD &= ~_BV(DDD2);
  DDRD &= ~_BV(DDD3);
  DDRD &= ~_BV(DDD6);
  DDRD &= ~_BV(DDD7);
  PORTD |= _BV(PORTD2) | _BV(PORTD3) | _BV(PORTD6) | _BV(PORTD7);

  // Screen Busy
  // -> Input
  DDRB &= ~_BV(DDB1);
  PORTB &= ~_BV(PORTB1);

  // Relay putput pins, outputs
  DDRB |= _BV(DDB2);
  PORTB &= ~_BV(PORTB2);
  DDRC |= _BV(DDC0) | _BV(DDC1) | _BV(DDC2);
  PORTC &= ~_BV(PORTC0);
  PORTC &= ~_BV(PORTC1);
  PORTC &= ~_BV(PORTC2);
}

void initPort() {}

void writePort(const uint8_t data, const uint8_t busyPin) {
  while (PINB & _BV(PINB1)) {
    ;
  }
  tw_write(data);
}

void hardReset() {}

void AvrDrivers::SetupScreen() {
  //
  Screen.GU7000_init();
}

void AvrDrivers::SetupTimer() {
  // prescaler clk / 1024
  TCCR1A |= _BV(CS12);
  TCCR1A &= ~_BV(CS11);
  TCCR1A |= _BV(CS10);

  // CTC mode on OCR1, to stop at that value
  // WGM1[3:0] = 0100 = 4
  TCCR1B &= ~_BV(WGM13);
  TCCR1B |= _BV(WGM12);
  TCCR1A &= ~_BV(WGM11);
  TCCR1A &= ~_BV(WGM10);

  // Set OCR1, approx ~10ms per overflow
  constexpr uint16_t top = 72;
  OCR1A = top;

  // Interrupt on compare A match
  TIMSK1 |= _BV(OCIE1A);
}

uint8_t dummy() {
  // We init IIC elsewhere, just pass this to the DS3231 to do nothing.
  return 0;
}

uint8_t iic_write(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
  uint8_t err = 0;
  err |= tw_master_transmit_one(addr, reg, true);
  err |= tw_master_transmit(addr, buf, len, false);
  return !(err == SUCCESS);
}

uint8_t iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
  uint8_t err = 0;
  err |= tw_master_transmit_one(addr, reg, true);
  err |= tw_master_receive(addr, buf, len);
  return !(err == SUCCESS);
}

void debug_print(const char *const fmt, ...) {}

void receive_callback(uint8_t type) {}

void delay_ms(uint32_t ms) {
  // Only used for reading temp, which we do not do, so get out of jail free
  // card.
}

uint8_t AvrDrivers::SetupRTC() {
  memset(&Rtc, 0, sizeof(Rtc));
  Rtc.iic_init = dummy;
  Rtc.iic_deinit = dummy;
  Rtc.iic_write = iic_write;
  Rtc.iic_read = iic_read;
  Rtc.debug_print = debug_print;
  Rtc.receive_callback = receive_callback;
  Rtc.delay_ms = delay_ms;

  int res = ds3231_init(&Rtc);
  res |= ds3231_set_square_wave(&Rtc, ds3231_bool_t::DS3231_BOOL_TRUE);

  return res;
}

// https://www.etlcpp.com/debounce.html
const int BTN_DEBOUNCE_COUNT = 5;
const int BTN_HOLD_COUNT = 50;
const int BTN_REPEAT_COUNT = 200;
typedef etl::debounce<BTN_DEBOUNCE_COUNT, BTN_HOLD_COUNT, BTN_REPEAT_COUNT>
    BtnDebounce;

const int TEMP_DEBOUNCE_COUNT = 200;
const int TEMP_HOLD_COUNT = 1000;
const int TEMP_REPEAT_COUNT = 2000;
typedef etl::debounce<TEMP_DEBOUNCE_COUNT, TEMP_HOLD_COUNT, TEMP_REPEAT_COUNT>
    TmpDebounce;

void AvrDrivers::ReadInput() {
  static BtnDebounce upButton;
  static BtnDebounce downButton;
  static BtnDebounce fanOnOff;
  static TmpDebounce tempCoolOn;
  static TmpDebounce tempHeatOn;
  static TmpDebounce tempNone;

  uint8_t pind = PIND;

  if (upButton.add(pind & PIND3) && upButton.is_set()) {
    Callbacks_.OnButtonPressed(Button::Up, UserData_);
  }

  if (downButton.add(pind & PIND2) && downButton.is_set()) {
    Callbacks_.OnButtonPressed(Button::Down, UserData_);
  }

  if (fanOnOff.add(pind & PIND6)) {
    if (fanOnOff.is_set()) {
      Callbacks_.OnButtonPressed(Button::FanOn, UserData_);
    } else {
      Callbacks_.OnButtonPressed(Button::FanAuto, UserData_);
    }
  }

  if (tempHeatOn.add(pind & PIND4) && tempHeatOn.is_set()) {
    Callbacks_.OnButtonPressed(Button::TempHeat, UserData_);
  }

  if (tempCoolOn.add(pind & PIND5) && tempCoolOn.is_set()) {
    Callbacks_.OnButtonPressed(Button::TempCold, UserData_);
  }

  if (tempNone.add(!tempCoolOn.is_set() && !tempHeatOn.is_set() &&
                   tempNone.is_held())) {
    Callbacks_.OnButtonPressed(Button::TempNone, UserData_);
  }
}
