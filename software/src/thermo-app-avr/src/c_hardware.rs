use thermo_app::hardware::IHardware;

#[repr(C)]
pub struct CHardwareDrivers {
    pub read_temp_c_function: extern "C" fn() -> u8,
}

pub struct CHardware {
    pub c: *const CHardwareDrivers,
}

impl CHardware {
    pub fn new(hw: *const CHardwareDrivers) -> Self {
        Self { c: hw }
    }

    fn get_hw(&self) -> &CHardwareDrivers {
        return unsafe { self.c.as_ref() }.unwrap();
    }
}

impl IHardware for CHardware {
    fn read_temperature(&self) -> u8 {
        (self.get_hw().read_temp_c_function)()
    }

    fn screen_write_temperature(&self, temp: u8) {
        todo!()
    }

    fn relay_on(&self, e: thermo_app::hardware::Relay) {
        todo!()
    }

    fn relay_off(&self, e: thermo_app::hardware::Relay) {
        todo!()
    }

    fn screen_write_setpoint(&self, set_point: u8) {
        todo!()
    }

    fn report_idle(&self) {
        todo!()
    }

    fn report_cooling(&self) {
        todo!()
    }

    fn report_heating(&self) {
        todo!()
    }

    fn report_temperature(&self, new_temp: u8) {
        todo!()
    }
}
