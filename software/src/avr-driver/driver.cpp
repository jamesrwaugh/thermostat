#include "driver.hpp"

#include <HardwareSerial.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <driver_ds1307.h>
#include <twi_master.h>
#include <util/delay.h>

#include "c_api/checksum.hpp"
#include "c_api/driver_rs_wrapper.hpp"

etl::optional<AvrDrivers> gDriver;

AvrDrivers::AvrDrivers() : Serial_(Serial) {}

void AvrDrivers::Setup() {
  SetupPins();
  SetupI2C();
  SetupInputTimer();
  SetupRTC();
  SetupScreen();
  SetupTemp();
  SetupFlash();
  SetupSerial();
  sei();
}

void AvrDrivers::SetupI2C() {
  // initialize twi prescaler and bit rate (250k)
  // Set prescaler value of 1
  TWSR &= ~_BV(TWPS0);
  TWSR &= ~_BV(TWPS1);
  TWBR = ((F_CPU / 250000) - 16) / 2;

  // Possible delay needed with setting up I2C?
  // Possible simulator bug only, *sometimes* screen initializes
  // without first sending I2C start message without this.
  _delay_ms(5);
}

void AvrDrivers::SetupPins() {
  // Up button, down button, cooling, heating, select
  // -> Input pullup
  DDRD &= ~_BV(DDD2);
  DDRD &= ~_BV(DDD3);
  DDRD &= ~_BV(DDD5);
  DDRD &= ~_BV(DDD6);
  DDRD &= ~_BV(DDD7);
  PORTD |= _BV(PORTD2) | _BV(PORTD3) | _BV(PORTD5) | _BV(PORTD6) | _BV(PORTD7);

  // Reverse Valve SIP Switch Input Pullup
  DDRC &= ~_BV(DDC3);
  PORTC |= _BV(PORTC3);

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

void writePort(const uint8_t data) {
  while (PINB & _BV(PINB1)) {
    ;
  }
  tw_write(data);
}

void hardReset() {}

void AvrDrivers::SetupScreen() {
  AutoTwi t;
  Screen.GU7000_init();
}

void AvrDrivers::SetupInputTimer() {
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

uint8_t dummy() {
  // We init IIC elsewhere, just pass this to the DS3231 to do nothing.
  return 0;
}

uint8_t iic_write(uint8_t addr, uint8_t reg, uint8_t* buf, uint16_t len) {
  uint8_t err = 0;
  err |= tw_master_transmit_one(addr >> 1, reg, true);
  err |= tw_master_transmit(addr >> 1, buf, len, false);
  return !(err == SUCCESS);
}

uint8_t iic_read(uint8_t addr, uint8_t reg, uint8_t* buf, uint16_t len) {
  uint8_t err = 0;
  err |= tw_master_transmit_one(addr >> 1, reg, true);
  err |= tw_master_receive(addr >> 1, buf, len);
  return !(err == SUCCESS);
}

void debug_print(const char* const fmt, ...) {}

void delay_ms(uint32_t ms) {
  // Not used for DS1307
}

uint8_t AvrDrivers::SetupRTC() {
  memset(&Rtc, 0, sizeof(Rtc));
  Rtc.iic_init = dummy;
  Rtc.iic_deinit = dummy;
  Rtc.iic_write = iic_write;
  Rtc.iic_read = iic_read;
  Rtc.debug_print = debug_print;
  Rtc.delay_ms = delay_ms;

  int res = ds1307_init(&Rtc);

  return res;
}

void AvrDrivers::SetupSerial() {
  Serial_.begin();
}

void AvrDrivers::SetupFlash() {
  // Need to use A2 and A1 here to avoid clash with 0x50
  // I2C Address of the screen
  ram_.begin(1, 1);
}

struct DebounceState {
  uint8_t ZeroCount{0};
  bool IsSet{false};

  bool Add(bool sample) {
    bool changed = false;

    if (!sample) {
      if (ZeroCount < 2) {
        ZeroCount += 1;
      } else if (!IsSet) {
        IsSet = true;
        changed = true;
      }
    } else {
      IsSet = false;
      ZeroCount = 0;
    }

    return changed;
  };
};

DebounceState UpButton;
DebounceState DownButton;
DebounceState SelectButton;
DebounceState FanButton;
DebounceState HeatButton;

int8_t AvrDrivers::ReadInput() {
  uint8_t pind = PIND;
  int8_t button = -1;

  if (UpButton.Add(pind & _BV(PIND3))) {
    button = static_cast<int8_t>(Button::Up);
  } else if (DownButton.Add(pind & _BV(PIND2))) {
    button = static_cast<int8_t>(Button::Down);
  } else if (SelectButton.Add(pind & _BV(PIND5))) {
    button = static_cast<int8_t>(Button::Select);
  } else if (HeatButton.Add(pind & _BV(PIND7))) {
    button = static_cast<int8_t>(Button::Heat);
  } else if (FanButton.Add(pind & _BV(PIND6))) {
    button = static_cast<int8_t>(Button::Fan);
  }

  return button;
}

void AvrDrivers::ReadStateNow(ThermoButtonState* data) const {
  bool reverseHeat = (PINC & _BV(PINC2)) != 0;
  data->ReverseValveState = reverseHeat ? ReverseValveModeT::OnForHeating
                                        : ReverseValveModeT::OnForCooling;
}

void AvrDrivers::RelayOn(Relay r) const {
  switch (r) {
    case Relay::Fan:
      PORTC |= _BV(PORTC0);
      break;
    case Relay::Compressor:
      PORTB |= _BV(PORTB2);
      break;
    case Relay::Heat:
      PORTC |= _BV(PORTC1);
      break;
    case Relay::ReversingValve:
      PORTC |= _BV(PORTC2);
      break;
  }
}

void AvrDrivers::RelayOff(Relay r) const {
  switch (r) {
    case Relay::Fan:
      PORTC &= ~_BV(PORTC0);
      break;
    case Relay::Compressor:
      PORTB &= ~_BV(PORTB2);
      break;
    case Relay::Heat:
      PORTC &= ~_BV(PORTC1);
      break;
    case Relay::ReversingValve:
      PORTC &= ~_BV(PORTC2);
      break;
  }
}

void AvrDrivers::SetupTemp() {
  TempSensor.Init();
}

constexpr uint16_t FlashSaveDataAddress = 0;

bool AvrDrivers::SaveData(const ThermoSaveData& data) const {
  uint8_t buffer[BYTES_LENGTH_THERMO_SAVE_DATA];
  EncodeThermoSaveData(&const_cast<ThermoSaveData&>(data), buffer);
  return WriteFlash(FlashSaveDataAddress, buffer,
                    BYTES_LENGTH_THERMO_SAVE_DATA);
}

bool AvrDrivers::LoadData(ThermoSaveData& data) const {
  uint8_t buffer[BYTES_LENGTH_THERMO_SAVE_DATA];

  if (!ReadFlash(FlashSaveDataAddress, buffer, BYTES_LENGTH_THERMO_SAVE_DATA)) {
    return false;
  }

  DecodeThermoSaveData(&data, buffer);

  return checksum(buffer + 1, sizeof(buffer) - 1) == data.checksum;
}

bool AvrDrivers::WriteFlash(uint16_t address, const uint8_t* data,
                            uint8_t length) const {
  uint8_t err = ram_.write(address, data, static_cast<uint16_t>(length));
  return err == 0;
}

bool AvrDrivers::ReadFlash(uint16_t address, uint8_t* buffer,
                           uint8_t maxLength) const {
  ram_.read(address, buffer, maxLength);
  return true;
}
