use thermo_app::hardware::IHardware;

#[repr(C)]
pub struct CHardwareDrivers {
    pub read_temp_c_function: extern "C" fn() -> u8,
}

pub struct CHardware<'a> {
    pub c: &'a CHardwareDrivers,
}

impl<'a> CHardware<'a> {
    pub fn new(hw: &'a CHardwareDrivers) -> Self {
        Self { c: hw }
    }
}

impl<'a> IHardware for CHardware<'a> {
    fn read_temperature(&self) -> u8 {
        (self.c.read_temp_c_function)()
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
