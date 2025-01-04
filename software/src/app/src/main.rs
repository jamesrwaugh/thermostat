#![no_std]
#![no_main]

mod avr_driver_api;
mod avr_hardware;
mod engine;
mod hardware;

use avr_driver_api::{AvrDriverCallbacks, Button, DriverInit};
use avr_hardware::AvrHardware;
use core::{ffi::c_void, panic::PanicInfo};
use engine::{Event, Thermostat};
use statig::prelude::{IntoStateMachineExt, StateMachine};

fn get_fsm<'a>(user_data: *mut ::core::ffi::c_void) -> &'a mut StateMachine<Thermostat<'a>> {
    let x = unsafe {
        (user_data as *mut StateMachine<Thermostat>)
            .as_mut()
            .unwrap()
    };
    x
}

extern "C" fn on_button_pressed(b: Button, user_data: *mut ::core::ffi::c_void) {
    let x = get_fsm(user_data);
    x.handle(&Event::UpButtonPressed);
}

extern "C" fn on_serial_message(
    message: *const ::core::ffi::c_char,
    messageLen: u16,
    user_data: *mut ::core::ffi::c_void,
) {
}

extern "C" fn on_second_passed(user_data: *mut ::core::ffi::c_void) {
    let x = get_fsm(user_data);
    x.handle(&Event::SecondPassed);
}

#[no_mangle]
pub extern "C" fn main() {
    let callbacks = AvrDriverCallbacks {
        OnButtonPressed: Some(on_button_pressed),
        OnSerialMessage: Some(on_serial_message),
        OnSecondPassed: Some(on_second_passed),
    };

    let hw = AvrHardware::default();

    let mut machine = Thermostat::new(&hw).state_machine();

    unsafe {
        let obj_ptr: *mut c_void = &mut machine as *mut _ as *mut c_void;
        DriverInit(&callbacks, obj_ptr);
    }

    loop {}
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
