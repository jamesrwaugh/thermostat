pub trait IHardware {
    fn read_temperature(&self) -> u8;
    fn screen_write_temperature(&self, temp: u8);
}
