#[repr(C)]
pub enum HeatSetting {
    Heating,
    Cooling,
}

#[repr(C)] // Ensure the struct has a C-compatible layout.
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

    pub fn on_second_passed() {}
}

pub fn add(left: usize, right: usize) -> usize {
    left + right
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn exploration() {
        let result = add(2, 2);
        assert_eq!(result, 4);
    }
}
