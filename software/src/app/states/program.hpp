#pragma once

#include "protos/ThermoSaveData_bp.h"
#include "state.hpp"

class Machine;

class Program : public State::Base {
 public:
  Program(Machine& machine);
  virtual ~Program();
  State::Type handle_event(const Event::Base& event) override;

 private:
  Machine& machine_;
  enum class Screen { TempDisplayType = 0 };
  Screen Screen_;
  ThermoSaveData DirtyData_;
};