use crate::hardware::IHardware;

#[repr(C)]
pub enum HeatSetting {
    Heating,
    Cooling,
}

#[repr(C)]
pub struct State {
    pub set_point: u8,
    pub current_temperature_c: u8,
    pub mode: HeatSetting,
    pub fan_warmup_time_sec: u8,
}

impl State {
    pub fn new() -> Self {
        Self {
            set_point: 70,
            current_temperature_c: 70,
            mode: HeatSetting::Cooling,
            fan_warmup_time_sec: 0,
        }
    }

    pub fn on_second_passed(&self, hw: &impl IHardware) {}

    fn on_cooling_mode_changed(mode: HeatSetting) {}
}
