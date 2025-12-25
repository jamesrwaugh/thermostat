#pragma once

#include "protos/ThermoSaveData_bp.h"
#include "state.hpp"

class Machine;

class Program : public State::Base {
 public:
  Program(Machine& machine);
  virtual ~Program();
  State::Type handle_event(const Event::Base& event) override;

  struct SelectionBox {
    const uint8_t XPositionDots{0};
    const uint8_t CharacterStride{0};
    const char* CharacterSet{nullptr};
    const uint8_t CharacterSetCount{0};
    void Draw(uint8_t characterIndex) const;
  };

 private:
  // 112 x 16
  // Temp Display Unit
  // Date Set
  // Time Set
  enum class Screen : uint8_t {
    TempDisplayUnit = 0,
    DateSet = 1,
    TimeSet = 2,
  };

  void EnterTemp(Noritake_VFD_GU7000& s);

  Machine& machine_;
  Screen Screen_;
  ThermoSaveData DirtyData_;
  uint8_t SelectedCharacterIndex{0};
  bool LineOn{false};
};