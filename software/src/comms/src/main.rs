#![no_std]
#![no_main]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use esp_hal::{
    clock::CpuClock,
    gpio::{Level, Output, OutputConfig},
    main,
    time::{Duration, Instant},
    uart::{Config, Uart},
};

#[main]
fn main() -> ! {
    let config = esp_hal::Config::default().with_cpu_clock(CpuClock::max());

    let peripherals = esp_hal::init(config);

    let mut led = Output::new(peripherals.GPIO0, Level::High, OutputConfig::default());

    let mut item = HaCommand {
        topic_key: MQTT_PING_TOPIC as u8,
        checksum: MQTT_PING_TOPIC as u8 + 1,
        payload_byte_two: 1,
        payload_byte_one: 0,
    };

    let config = Config::default().with_baudrate(9600);

    let mut uart = Uart::new(peripherals.UART0, config).unwrap();

    unsafe {
        let mut rx_buffer = [0u8; BYTES_LENGTH_HA_COMMAND as usize];
        EncodeHaCommand(&mut item, rx_buffer.as_mut_ptr());
        uart.write(&rx_buffer).unwrap_or(0);
    }

    loop {
        led.toggle();
        let delay_start = Instant::now();
        while delay_start.elapsed() < Duration::from_millis(500) {}
    }
}

#[panic_handler]
fn panic(_: &core::panic::PanicInfo) -> ! {
    esp_hal::system::software_reset()
}
