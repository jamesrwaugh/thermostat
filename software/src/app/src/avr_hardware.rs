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

    fn read_temperature(&self) -> u8 {
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
