#include "machine.hpp"
#include "protos/ThermoSaveData_bp.h"
#include "state.hpp"

class Program : public State::Base {
 public:
  Program(Machine& machine);
  ~Program();
  State::Type::TheType handle_event(const Event::Base& event) override;

 private:
  Machine& machine_;
  enum class Screen { TempDisplayType = 0 };
  Screen Screen_;
  ThermoSaveData DirtyData_;
};