use statig::prelude::{IntoStateMachineExt, StateMachine};
use thermo_app::engine::{Event, Thermostat};

use crate::c_hardware::{CHardware, CHardwareDrivers};

static mut MACHINE: Option<StateMachine<Thermostat>> = None;
static mut DRIVERS: Option<CHardware> = None;

#[no_mangle]
pub extern "C" fn ThermoInit(drivers: *const CHardwareDrivers) {
    unsafe {
        DRIVERS = Some(CHardware::new(drivers));
    }
    unsafe { MACHINE = Some(Thermostat::new(DRIVERS.as_ref().unwrap()).state_machine()) };
}

#[no_mangle]
pub extern "C" fn ThermoGetSetPoint() -> u8 {
    if let Some(o) = unsafe { MACHINE.as_mut() } {
        o.set_point
    } else {
        0
    }
}

#[no_mangle]
pub extern "C" fn ThermoUpButtonPressed() {
    if let Some(o) = unsafe { MACHINE.as_mut() } {
        o.handle(&Event::UpButtonPressed)
    }
}

#[no_mangle]
pub extern "C" fn ThermoSecondPassed() {
    if let Some(o) = unsafe { MACHINE.as_mut() } {
        o.handle(&Event::SecondPassed);
    }
}
