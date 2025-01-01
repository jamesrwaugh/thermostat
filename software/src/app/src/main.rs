#![no_std]
#![no_main]

use core::panic::PanicInfo;

#[link(name = "snappy")]
extern "C" {
    fn snappy_max_compressed_length(source_length: u16) -> u16;
}

#[no_mangle]
pub extern "C" fn main() {
    let x = unsafe { snappy_max_compressed_length(100) };
    loop {}
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
