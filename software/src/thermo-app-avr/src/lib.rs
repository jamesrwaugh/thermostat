#![no_std]
#![no_main]

use core::panic::PanicInfo;

#[no_mangle]
pub extern "C" fn RustAdd2(a: i32, b: i32) -> i32 {
    return a + b;
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
