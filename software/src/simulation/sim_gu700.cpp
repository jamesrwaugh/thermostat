#include "sim_gu700.hpp"

enum CommandId : uint8_t {
  Temp = 0,
  SetPoint = 1,
  IsHeating = 2,
  IsCooling = 3,
  IsIdle = 4,
};

SimFakeGu7000::SimFakeGu7000(avr_t* avr)
    : SimAvrI2CSmarterComponent(avr, 0x32) {}

void SimFakeGu7000::OnDataReceived(const std::vector<uint8_t>& data) {
  if (data.empty()) {
    return;
  }

  if (data.at(0) == CommandId::Temp) {
    State_.Temp = data.at(1);
  } else if (data.at(0) == CommandId::SetPoint) {
    State_.SetPoint = data.at(1);
  } else if (data.at(0) == CommandId::IsHeating) {
    State_.CoolState = CoolStateE::Heating;
  } else if (data.at(0) == CommandId::IsCooling) {
    State_.CoolState = CoolStateE::Cooling;
  } else if (data.at(0) == CommandId::IsIdle) {
    State_.CoolState = CoolStateE::Idle;
  }
}

const SimFakeGu7000::State& SimFakeGu7000::GetState() const {
  return State_;
}
