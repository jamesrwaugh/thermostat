#![no_std]
#![no_main]

use esp_backtrace as _;
use esp_hal::{
    delay::Delay,
    gpio::{Level, Output, OutputConfig},
    main,
};
use esp_println::println;

esp_bootloader_esp_idf::esp_app_desc!();

/// Onboard LED on many ESP32-C3 dev boards (GPIO 8). Use GPIO7 for ESP32-C3-DevKit-RUST-1.
#[main]
fn main() -> ! {
    let peripherals = esp_hal::init(esp_hal::Config::default());

    println!("comms: blink starting");

    let mut led = Output::new(peripherals.GPIO8, Level::Low, OutputConfig::default());

    let delay = Delay::new();

    loop {
        led.toggle();
        delay.delay_millis(500);
    }
}
