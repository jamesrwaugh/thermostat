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
    fn relay_on(&self, e: Relay);
    fn relay_off(&self, e: Relay);
}
