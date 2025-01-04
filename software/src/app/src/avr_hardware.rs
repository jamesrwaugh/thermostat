use crate::hardware::IHardware;

#[derive(Default)]
pub struct AvrHardware;

impl IHardware for AvrHardware {
    fn screen_write_temperature(&self, temp: u8) {
        todo!()
    }

    fn screen_write_setpoint(&self, set_point: u8) {
        todo!()
    }

    fn relay_on(&self, e: crate::hardware::Relay) {
        todo!()
    }

    fn relay_off(&self, e: crate::hardware::Relay) {
        todo!()
    }
}
