#![no_std]
#![no_main]

// Required to re-export C API for the AVR target
#[allow(unused_imports)]
use thermo_app;

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
