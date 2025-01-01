#[repr(C)]
#[derive(PartialEq)]
pub enum Relay {
    Fan,
    Compressor,
    Heat,
    ReversingValve,
}

pub trait IHardware {
    fn read_temperature(&self) -> u8;
    fn screen_write_temperature(&self, temp: u8);
    fn screen_write_setpoint(&self, set_point: u8);
    fn relay_on(&self, e: Relay);
    fn relay_off(&self, e: Relay);
    fn report_idle(&self);
    fn report_cooling(&self);
    fn report_heating(&self);
    fn report_temperature(&self, new_temp: u8);
}

#[repr(C)]
pub struct IHardware2 {
    pub read_temperature: fn() -> u8,
}
