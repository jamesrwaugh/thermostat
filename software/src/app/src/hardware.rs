pub enum Relay {
    Fan,
    Compressor,
    Heat,
    ReversingValve,
}

pub trait IHardware {
    fn screen_write_temperature(&self, temp: u8);
    fn screen_write_setpoint(&self, set_point: u8);
    fn relay_on(&self, e: Relay);
    fn relay_off(&self, e: Relay);
}
