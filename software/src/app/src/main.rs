#![no_std]
#![no_main]
#![feature(offset_of)]

mod avr_driver_api;
mod avr_hardware;
mod engine;
mod hardware;

use avr_driver_api::{AvrDriverCallbacks, Button, DriverInit, DriverMcuSleep};
use avr_hardware::AvrHardware;
use core::{ffi::c_void, panic::PanicInfo};
use engine::{Event, FanSetting, HeatSetting, Thermostat};
use statig::prelude::{IntoStateMachineExt, StateMachine};

fn get_fsm<'a>(user_data: *mut ::core::ffi::c_void) -> &'a mut StateMachine<Thermostat<'a>> {
    let fsm = unsafe {
        (user_data as *mut StateMachine<Thermostat>)
            .as_mut()
            .unwrap()
    };
    fsm
}

extern "C" fn on_button_pressed(button: Button, user_data: *mut ::core::ffi::c_void) {
    let fsm = get_fsm(user_data);
    match button {
        Button::Up => fsm.handle(&Event::UpButtonPressed),
        Button::Down => fsm.handle(&Event::DownButtonPressed),
        Button::TempHeat => fsm.handle(&Event::CoolingModeChanged(HeatSetting::Heating)),
        Button::TempCold => fsm.handle(&Event::CoolingModeChanged(HeatSetting::Cooling)),
        Button::TempNone => fsm.handle(&Event::CoolingModeChanged(HeatSetting::None)),
        Button::FanAuto => fsm.handle(&Event::FanSettingChanged(FanSetting::Auto)),
        Button::FanOn => fsm.handle(&Event::FanSettingChanged(FanSetting::On)),
    };
}

extern "C" fn on_serial_message(
    _message: *const ::core::ffi::c_char,
    _messageLen: u16,
    _user_data: *mut ::core::ffi::c_void,
) {
}

extern "C" fn on_second_passed(user_data: *mut ::core::ffi::c_void) {
    let fsm = get_fsm(user_data);
    fsm.handle(&Event::SecondPassed);
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

    loop {
        unsafe {
            DriverMcuSleep();
        }
    }
}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}
