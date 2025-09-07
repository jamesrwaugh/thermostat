#include "ui.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <string_view>

#include "ftxui-toolbox/logs_renderer.hpp"

Ui::Ui(SimAvrThermostat& thermostat, LockedDequeue& logs)
    : Thermostat_(thermostat) {
  using namespace ftxui;

  // Left Panel - Controls
  auto up = Button(
      "Up", [&] { Thermostat_.PushUpButton(); }, ButtonOption::Animated());

  auto down = Button(
      "Down", [&] { Thermostat_.PushDownButton(); }, ButtonOption::Animated());

  auto select = Button(
      "Select", [&] { Thermostat_.PushSelectButton(); },
      ButtonOption::Animated());

  // Fan switch (On/Auto) - using buttons since Toggle isn't available
  auto fan_auto_button = Button(
      "Fan: Auto",
      [&] {
        fan_selection_ = 0;
        Thermostat_.SwitchFanAuto();
      },
      ButtonOption::Animated());

  auto fan_on_button = Button(
      "Fan: On",
      [&] {
        fan_selection_ = 1;
        Thermostat_.SwitchFanOn();
      },
      ButtonOption::Animated());

  // Heat switch (Heat/Cool/None) - using buttons for three options
  auto heat_heat_button = Button(
      "Heat",
      [&] {
        heat_selection_ = 0;
        Thermostat_.SwitchHeatingHeat();
      },
      ButtonOption::Animated());

  auto heat_cool_button = Button(
      "Cool",
      [&] {
        heat_selection_ = 1;
        Thermostat_.SwitchHeatingCooling();
      },
      ButtonOption::Animated());

  auto heat_none_button = Button(
      "None",
      [&] {
        heat_selection_ = 2;
        Thermostat_.SwitchHeatingNone();
      },
      ButtonOption::Animated());

  auto topLabel = Renderer([] { return text("Buttons") | bold; });
  auto fan_label = Renderer([] { return text("Fan Mode:") | bold; });
  auto heat_label = Renderer([] { return text("Heat Mode:") | bold; });
  auto separator0 = Renderer([] { return separator(); });
  auto separator1 = Renderer([] { return separator(); });
  auto separator2 = Renderer([] { return separator(); });

  auto left_panel = Container::Vertical({
      topLabel,
      separator0,
      up,
      down,
      select,
      separator1,
      fan_label,
      fan_auto_button,
      fan_on_button,
      separator2,
      heat_label,
      heat_heat_button,
      heat_cool_button,
      heat_none_button,
  });

  // Middle Panel - Relay States
  auto relay_panel = Renderer([this] {
    const auto& relay_state = Thermostat_.GetRelayState();

    auto fan_light = relay_state.Fan ? text("●") | color(Color::Green)
                                     : text("●") | color(Color::GrayDark);

    auto compressor_light = relay_state.Compressor
                                ? text("●") | color(Color::Green)
                                : text("●") | color(Color::GrayDark);

    auto heat_light = relay_state.Heat ? text("●") | color(Color::Green)
                                       : text("●") | color(Color::GrayDark);

    auto reverse_valve_light = relay_state.ReverseValve
                                   ? text("●") | color(Color::Green)
                                   : text("●") | color(Color::GrayDark);

    return vbox({
        text("Relay States") | bold | center,
        separator(),
        hbox({
            text("Fan"),
            fan_light | center,
        }),
        hbox({
            text("Compressor"),
            compressor_light | center,
        }),
        hbox({
            text("Heat"),
            heat_light | center,
        }),
        hbox({
            text("Reverse Valve"),
            reverse_valve_light | center,
        }),
    });
  });

  // Right Panel - Screen State
  auto screen_panel = Renderer([this] {
    const auto& screen_state = Thermostat_.GetScreenState();

    std::string_view cool_state_str;
    switch (screen_state.CoolState) {
      case SimFakeGu7000::CoolStateE::Idle:
        cool_state_str = "Idle";
        break;
      case SimFakeGu7000::CoolStateE::Cooling:
        cool_state_str = "Cooling";
        break;
      case SimFakeGu7000::CoolStateE::Heating:
        cool_state_str = "Heating";
        break;
    }

    return vbox({
        text("Screen State") | bold | center,
        separator(),
        vbox({
            hbox({text("State: "), text(cool_state_str.data()) | bold}),
            hbox({text("Set Point: "),
                  text(std::to_string(screen_state.SetPoint)) | bold}),
            hbox({text("Temperature: "),
                  text(std::to_string(screen_state.Temp)) | bold}),
        }) | border,
    });
  });

  auto logs_panel = LogsRenderer(logs);

  // Main layout - three panels side by side
  auto main_layout = Container::Horizontal({
      left_panel,
      relay_panel,
      screen_panel,
      logs_panel,
  });

  Add(main_layout);
}
