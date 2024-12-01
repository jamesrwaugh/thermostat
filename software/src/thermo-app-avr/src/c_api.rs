use statig::prelude::{IntoStateMachineExt, StateMachine};
use thermo_app::{
    engine::{Event, FakeHw, Thermostat},
    hardware::IHardware,
};

use crate::c_hardware::{CHardware, CHardwareDrivers};

#[repr(C)] // Ensure the struct has a C-compatible layout.
pub struct CState<'a> {
    pub state: u8,
    pub driver: &'a CHardwareDrivers,
}

static mut MACHINE: Option<StateMachine<Thermostat>> = None;

impl<'a> CState<'a> {
    pub fn new(drivers: &'a CHardwareDrivers) -> Self {
        Self {
            state: 0,
            driver: drivers,
        }
    }
}

#[no_mangle]
pub extern "C" fn ThermoInit(state: *mut CState, drivers: &CHardwareDrivers) {
    unsafe { MACHINE = Some(Thermostat::new(drivers).state_machine()) };
    unsafe {
        if let Some(drivers) = drivers.as_mut() {
            let obj = state.as_mut().unwrap();
            *obj = CState::new(drivers);
        }
    }
}

#[no_mangle]
pub extern "C" fn ThermoGetSetPoint(obj: *mut CState) -> u8 {
    if let Some(obj) = unsafe { obj.as_mut() } {
        0
    } else {
        0
    }
}

#[no_mangle]
pub extern "C" fn ThermoUpButtonPressed(obj: *mut CState) {
    if let Some(obj) = unsafe { obj.as_mut() } {}
}

#[no_mangle]
pub extern "C" fn ThermoSecondPassed(obj: *mut CState) {
    if let Some(obj) = unsafe { obj.as_mut() } {
        let hw = CHardware::new(&obj.driver);

        let board = unsafe { &mut MACHINE };

        if let Some(ref mut e) = board {
            e.handle_with_context(&Event::SecondPassed, hw);
        }
    }
}
