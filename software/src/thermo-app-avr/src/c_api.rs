use thermo_app::engine::State;

use crate::c_hardware::{CHardware, CHardwareDrivers};

#[repr(C)] // Ensure the struct has a C-compatible layout.
pub struct CState<'a> {
    pub state: State,
    pub driver: &'a CHardwareDrivers,
}

impl<'a> CState<'a> {
    pub fn new(drivers: &'a CHardwareDrivers) -> Self {
        Self {
            state: State::new(),
            driver: drivers,
        }
    }
}

#[no_mangle]
pub extern "C" fn ThermoInit(state: *mut CState, drivers: *mut CHardwareDrivers) {
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
        obj.state.set_point
    } else {
        0
    }
}

#[no_mangle]
pub extern "C" fn ThermoSetSetPoint(obj: *mut CState, set_point: u8) {
    if let Some(obj) = unsafe { obj.as_mut() } {
        obj.state.set_point = set_point;
    }
}

#[no_mangle]
pub extern "C" fn ThermoSecondPassed(obj: *mut CState) {
    if let Some(obj) = unsafe { obj.as_mut() } {
        let hw = CHardware::new(&obj.driver);
        obj.state.on_second_passed(&hw)
    }
}
