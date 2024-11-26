enum HeatSetting {
    Heating,
    Cooling,
}

struct State {
    pub set_point: u8,
    pub current_temperature_c: u8,
    pub mode: HeatSetting,
    pub fan_warmup_time_sec: u8,
}

impl State {
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
