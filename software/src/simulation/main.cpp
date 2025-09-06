#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/receiver.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/task.hpp>

#include "sim_thermostat.hpp"
#include "ui.hpp"

void sim_debug_log(const char* fmt, va_list args) {
  va_list args2;
  va_copy(args2, args);

  int len = 1 + vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  std::string message(len, 0);
  vsnprintf(message.data(), message.length(), fmt, args2);
  va_end(args2);
}

void UiThread(SimAvrThermostat& thermo,
              std::atomic<bool>& refresh_ui_continue) {
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  auto top = ftxui::Make<Ui>(thermo);

  auto topWCatch = CatchEvent(top, [&](ftxui::Event event) {
    if (event == ftxui::Event::Character('q') ||
        event == ftxui::Event::Escape) {
      screen.Exit();
      refresh_ui_continue = false;
      return true;
    }
    return false;
  });

  ftxui::Loop loop(&screen, topWCatch);

  while (refresh_ui_continue) {
    loop.RunOnce();
    screen.Post(ftxui::Event::Custom);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

int main() {
  set_sim_debug_log(sim_debug_log);

  std::atomic<bool> refresh_ui_continue = true;
  auto rec = ftxui::MakeReceiver<ftxui::Closure>();

  std::string_view firmware_path =
      "/home/james/Desktop/Git/squaredel/thermostat/software/builddir/src/app/"
      "thermo-app";

  SimAvrThermostat thermo(firmware_path, false, rec);

  auto ui_thread =
      std::thread(UiThread, std::ref(thermo), std::ref(refresh_ui_continue));

  thermo.BlockingLoop(refresh_ui_continue, rec);

  ui_thread.join();

  return 0;
}