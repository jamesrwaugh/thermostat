use crate::hardware::{IHardware, Relay};

#[repr(C)]
#[derive(PartialEq)]
pub enum HeatSetting {
    Heating,
    Cooling,
}

#[repr(C)]
#[derive(PartialEq)]
enum CoolState {
    Idle,
    Active,
}

#[repr(C)]
pub struct State {
    pub set_point: u8,
    pub current_temperature_c: u8,
    pub mode: HeatSetting,
    pub fan_warmup_time_sec: u8,
    state: CoolState,
    setpoint_hit_time_sec: u8,
}

// Idle:
// - If cooling and too hot for some time, go to active
// - If hot and too cold  for some time, go to active
// Active:
// - Turn on fan
// - Activate proper relays and go to setpoint
// - If close to setpoint for some time,
//   - Wait for a bit, then go to idle.
// - If changed to opposite mode:
//   - Do nothing... we should get out of Active in a little bit

impl State {
    pub fn new() -> Self {
        Self {
            set_point: 70,
            current_temperature_c: 70,
            mode: HeatSetting::Cooling,
            fan_warmup_time_sec: 0,
            state: CoolState::Idle,
            setpoint_hit_time_sec: 0,
        }
    }

    pub fn on_second_passed(&self, hw: &impl IHardware) {
        let temp = hw.read_temperature();
        hw.screen_write_temperature(temp);
    }

    fn handle_idle(&self) {}

    fn active_enter(&self, hw: &impl IHardware) {
        hw.relay_on(Relay::Fan);
        hw.relay_on(Relay::ReversingValve);
    }

    fn active_exit(&self, hw: &impl IHardware) {
        hw.relay_off(Relay::Fan);
        hw.relay_off(Relay::Compressor);
        hw.relay_off(Relay::Heat);
        hw.relay_off(Relay::ReversingValve);
    }

    fn handle_active(&mut self, hw: &impl IHardware, temp: u8) {
        if self.set_point == temp {
            self.setpoint_hit_time_sec += 1;
            if self.setpoint_hit_time_sec >= 5 {
                // Go to idle
            }
        }
    }
}
