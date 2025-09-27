#include <avr/interrupt.h>
#include <bmodbus.h>
#include <driver_ds1307.h>
#include <string.h>

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
#include "state.hpp"
#include "states/machine.hpp"

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
    case Button::TempHeat:
      machine.receive(Event::HeatModeChanged(HeatModeT::Heating));
      break;
    case Button::TempCold:
      machine.receive(Event::HeatModeChanged(HeatModeT::Cooling));
      break;
    case Button::TempNone:
      machine.receive(Event::HeatModeChanged(HeatModeT::None));
      break;
    case Button::FanAuto:
      machine.receive(Event::FanModeChanged(FanModeT::Auto));
      break;
    case Button::FanOn:
      machine.receive(Event::FanModeChanged(FanModeT::On));
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

void PrintStateChange(const char* message) {
  const char* n = "New --> ";
  DriverWriteSerialPortRaw((uint8_t*)n, strlen(n));
  DriverWriteSerialPortRaw((uint8_t*)message, strlen(message));
  DriverWriteSerialPortRaw((uint8_t*)"\r\n", 2);
}

void OnStateChange(State::Type state) {
  switch (state) {
    case State::Type::Idle:
      PrintStateChange("ID");
      break;
    case State::Type::Heating:
      PrintStateChange("H");
      break;
    case State::Type::Cooling:
      PrintStateChange("C");
      break;
    case State::Type::Program:
      PrintStateChange("P");
      break;
    case State::Type::COUNT:
      PrintStateChange("?");
      break;
    default:
      PrintStateChange("?");
      break;
  }
}

uint16_t register_values[16] = {0};  // Example register values

static int dispatch(uint8_t function, uint16_t address, uint16_t* data,
                    uint8_t size) {
  // Handle the request based on the function code
  switch (function) {
    case 0x03:  // Read Holding Registers
    case 0x04:  // Read Input Registers
      // Handle read input registers request
      // memset(data, 0, size * 2); // Clear the data buffer
      if (address == 0x1234) {
        data[0] = 0xABCD;  // Example value for address 0x1234
      } else if (address == 0x5678) {
        data[0] = 0xEF01;  // Example value for address 0x5678
      } else if (address < 16) {
        for (uint8_t i = 0; i < size; i++) {
          if (i + address < sizeof(register_values) / 2) {
            data[i] = register_values[address + i];  // Copy register values
          } else {
            data[i] = 0;  // Fill with zero if out of bounds
          }
        }
      }
      break;
    case 0x06:  // Write Single Register
    case 0x10:  // Write Multiple Registers
      // Handle write single/multiple registers request
      if (address < 16) {
        for (int i = 0; i < size; i++) {
          if (i + address < sizeof(register_values) / 2) {
            register_values[address + i] =
                data[i];  // Store the value in the register
          }
        }
      } else {
        return 0;  // Invalid address
      }
      break;
    default:
      return 0;  // Unsupported function code
  }
  return 0;  // Success
}

void CheckModbus(modbus_client_t& mb, uint8_t tenMsCount) {
  uint8_t byte;
  if (DriverGetSerialByte(&byte)) {
    bmodbus_client_next_byte(&mb, tenMsCount * 10000, byte);
  }

  modbus_request_t* request = bmodbus_client_get_request(&mb);

  if (request) {
    if (dispatch(request->function, request->address, request->data,
                 request->size) == 0) {
      modbus_uart_data_t* r = bmodbus_client_get_response(&mb);
      DriverWriteSerialPortRaw(r->data, r->size);
    } else {
      request->result = -1;
      modbus_uart_data_t* r = bmodbus_client_get_response(&mb);
      DriverWriteSerialPortRaw(r->data, r->size);
    }
  }
}

int main() {
  AvrDriverCallbacks callbacks{
      .OnButtonPressed = OnButtonPressed,
  };

  DriverInit(callbacks);

  ds1307_time_s time = {
      .year = 2025,
      .month = 9,
      .week = 2,
      .date = 16,
      .hour = 6,
      .minute = 37,
      .second = 23,
      .am_pm = ds1307_am_pm_t::DS1307_PM,
  };

  DriverSetTime(time);

  machine.start();

  DriverGetTime(time);

  uint8_t lastTenMsCount = 0;

  DriverWriteSerialPortS("Hello, world!");

  // Invalid state to force initial update
  State::Type lastState = State::Type::COUNT;

  modbus_client_t mb;
  constexpr uint32_t interFrameDelay = INTERFRAME_DELAY_MICROSECONDS(9600);
  bmodbus_client_init(&mb, interFrameDelay, 34);

  while (true) {
    auto state = machine.get_state_id();

    if (lastState != state) {
      lastState = state;
      OnStateChange(state);
    }

    if (g10MillisecondPassed) {
      DriverPollInput();
      g10MillisecondPassed = false;
      lastTenMsCount += 1;
    }

    CheckModbus(mb, lastTenMsCount);

    if (lastTenMsCount >= 10) {
      lastTenMsCount = 0;
      machine.receive(Event::SecondPassed());
    }

    DriverMcuSleep();
  }

  return 0;
}

ISR(TIMER1_COMPA_vect) {
  g10MillisecondPassed = true;
}

void operator delete(void*, unsigned int) {}
