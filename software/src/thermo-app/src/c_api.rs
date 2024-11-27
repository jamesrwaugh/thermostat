use crate::setpoint::State;

#[no_mangle]
pub extern "C" fn ThermoInit(buffer: *mut State) {
    unsafe {
        // Initialize the memory buffer as a valid MyStruct instance.
        let obj = buffer.as_mut().unwrap();
        *obj = State::new();
    }
}

#[no_mangle]
pub extern "C" fn ThermoGetSetPoint(obj: *mut State) -> u8 {
    if let Some(obj) = unsafe { obj.as_mut() } {
        obj.set_point
    } else {
        0
    }
}

#[no_mangle]
pub extern "C" fn ThermoSetSetPoint(obj: *mut State, set_point: u8) {
    if let Some(obj) = unsafe { obj.as_mut() } {
        obj.set_point = set_point;
    }
}
