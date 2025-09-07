#include "SimulatedGu7000.hpp"

#include <twi_master.h>

enum CommandId : uint8_t {
  Temp = 0,
  SetPoint = 1,
  IsHeating = 2,
  IsCooling = 3,
  IsIdle = 4,
};

void SimulatedGu7000::DriverDisplayTemp(uint8_t temp) {
  uint8_t data[] = {CommandId::Temp, temp};
  tw_master_transmit(FakeI2cAddress, data, sizeof(data), false);
};

void SimulatedGu7000::DriverDisplaySetPoint(uint8_t temp) {
  uint8_t data[] = {CommandId::SetPoint, temp};
  tw_master_transmit(FakeI2cAddress, data, sizeof(data), false);
}

void SimulatedGu7000::DriverDisplayIsHeating() {
  uint8_t data[] = {CommandId::IsHeating};
  tw_master_transmit(FakeI2cAddress, data, sizeof(data), false);
}

void SimulatedGu7000::DriverDisplayIsCooling() {
  uint8_t data[] = {CommandId::IsCooling};
  tw_master_transmit(FakeI2cAddress, data, sizeof(data), false);
}

void SimulatedGu7000::DriverDisplayIsIdle() {
  uint8_t data[] = {CommandId::IsIdle};
  tw_master_transmit(FakeI2cAddress, data, sizeof(data), false);
}
