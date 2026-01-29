#include <Noritake_VFD_GU7000.h>
#include <avr/interrupt.h>
#include <driver_ds1307.h>
#include <mqtt.h>

#include <driver_rs_wrapper.hpp>

#include "event.hpp"
#include "states/machine.hpp"
#include "states/program_screen.hpp"

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
    case Button::Fan:
      machine.receive(Event::FanButtonPressed());
      break;
    case Button::Heat:
      machine.receive(Event::HeatButtonPressed());
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

void publish_response_callback(void** state,
                               struct mqtt_response_publish* publish) {}

int main() {
  DriverInit();
  machine.start();

  uint8_t lastHalfSecondCount = 0;
  uint8_t lastSecondCount = 0;

  ProgramScreenState::Screen_ = &DriverGetScreenHandle();
  ScreenBox::Screen_ = &DriverGetScreenHandle();

  uint8_t sendbuf[50];
  uint8_t recvbuf[50];

  struct mqtt_client client; /* instantiate the client */
  mqtt_init(&client, 0, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf),
            publish_response_callback); /* initialize the client */

  /* Send connection request to the broker. */
  const char* client_id = NULL;
  uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
  mqtt_connect(&client, client_id, NULL, NULL, 0, NULL, NULL, connect_flags,
               400);

  /* publish the time */
  const char* message = "Hello";
  mqtt_publish(&client, "temp", message, strlen(message) + 1, MQTT_PUBLISH_QOS_0);

  while (true) {
    if (g10MillisecondPassed) {
      auto button = DriverReadButton();
      if (button != -1) {
        OnButtonPressed(static_cast<Button>(button));
      }
      g10MillisecondPassed = false;
      lastHalfSecondCount += 1;
      lastSecondCount += 1;
    }

    if (lastHalfSecondCount >= 50) {
      lastHalfSecondCount = 0;
      machine.receive(Event::HalfSecondPassed());
    }

    if (lastSecondCount >= 100) {
      lastSecondCount = 0;
      machine.receive(Event::SecondPassed());
    }

    mqtt_sync(&client);

    DriverMcuSleep();
  }

  return 0;
}

ISR(TIMER1_COMPA_vect) {
  g10MillisecondPassed = true;
}

void operator delete(void*, unsigned int) {}
