#include "ui.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

Ui::Ui(SimAvrThermostat& thermostat) : Thermostat_(thermostat) {
  using namespace ftxui;

  auto b = Button(
      "Up", [&] { Thermostat_.PushUpButton(); }, ButtonOption::Animated());

  Add(b);
}
