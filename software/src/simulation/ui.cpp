#include "ui.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

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

  // Fan switch (On/Auto)
  auto fan_button = Button(
      "Fan", [&] { Thermostat_.PushFanButton(); }, ButtonOption::Animated());

  // Heat switch (Heat/Cool/None)
  auto heat_button = Button(
      "Heat", [&] { Thermostat_.PushHeatButton(); }, ButtonOption::Animated());

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
                        fan_button,
                        separator2,
                        heat_label,
                        heat_button,
                    }) |
                    size(ftxui::WIDTH, EQUAL, 20);

  auto screen =
      Renderer([&] {
        const auto& screen = Thermostat_.GetScreenMemory();

        auto c = Canvas(112, 16);

        for (unsigned y = 0; y < 16; y++) {
          for (unsigned x = 0; x < 112; x++) {
            c.DrawPoint(x, y, true,
                        [&](Pixel& s) { s.background_color = Color::Grey0; });
            c.DrawPoint(x, y, screen.at(x).at(y),
                        [&](Pixel& s) { s.foreground_color = Color::Cyan3; });
          }
        }

        return canvas(std::move(c)) | border;
      }) |
      size(ftxui::HEIGHT, EQUAL, 10);

  // Middle Panel - Relay States
  auto relay_panel =
      Renderer([this] {
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
      }) |
      size(ftxui::WIDTH, EQUAL, 32);

  auto logs_panel = LogsRenderer(logs);

  // I2C Listener Panel
  //   auto i2c_listener_panel = I2CListenerRenderer(Thermostat_.GetAvr()) |
  //                             size(ftxui::WIDTH, ftxui::LESS_THAN, 80);

  // Main layout - four panels side by side with horizontal divider
  auto bottom_items = Container::Horizontal({
                          left_panel,
                          relay_panel,
                          logs_panel,
                      }) |
                      size(ftxui::HEIGHT, ftxui::LESS_THAN, 40);

  auto main_layout = Container::Vertical({
      screen,
      bottom_items,
  });

  Add(main_layout);
}
