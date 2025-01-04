use crate::hardware::{IHardware, Relay};

use statig::{
    state_machine,
    Response::{self, Handled, Super, Transition},
};

#[derive(PartialEq, Clone)]
pub enum HeatSetting {
    None,
    Heating,
    Cooling,
}

#[derive(PartialEq, Clone)]
pub enum FanSetting {
    Auto,
    On,
}

pub struct Thermostat<'a> {
    pub set_point: u8,
    pub hw: &'a dyn IHardware,
    pub mode: HeatSetting,
    pub last_reported_temp: u8,
}

impl<'a> Thermostat<'a> {
    pub fn new(hw: &'a dyn IHardware) -> Self {
        Self {
            set_point: 80,
            hw,
            mode: HeatSetting::Cooling,
            last_reported_temp: 0,
        }
    }
}

pub enum Event {
    UpButtonPressed,
    DownButtonPressed,
    CoolingModeChanged(HeatSetting),
    FanSettingChanged(FanSetting),
    SecondPassed,
}

//  Idle:
//  - If cooling and too hot for some time, go to active
//  - If hot and too cold  for some time, go to active
//  Active:
//  - Turn on fan
//  - Activate proper relays and go to setpoint
//  - If close to setpoint for some time,
//    - Wait for a bit, then go to idle.
//  - If changed to opposite mode:
//    - Do nothing... we should get out of Active in a little bit

#[state_machine(initial = "State::idle(0)")]
impl<'a> Thermostat<'a> {
    #[state(entry_action = "idle_enter")]
    fn idle(&mut self, ambient_change_count: &mut u8, event: &Event) -> Response<State> {
        match event {
            Event::UpButtonPressed => {
                self.update_setpoint(true);
                let temp = self.hw.read_temperature();
                if temp > self.set_point && self.mode == HeatSetting::Heating {
                    Transition(State::heating(0))
                } else {
                    Handled
                }
            }
            Event::DownButtonPressed => {
                self.update_setpoint(true);
                let temp = self.hw.read_temperature();
                if temp < self.set_point && self.mode == HeatSetting::Cooling {
                    Transition(State::cooling(0))
                } else {
                    Handled
                }
            }
            Event::SecondPassed => {
                let temp = self.hw.read_temperature();
                self.check_report_temp(temp);
                if temp > self.set_point && self.mode == HeatSetting::Cooling {
                    *ambient_change_count += 1;
                    if *ambient_change_count >= 5 {
                        return Transition(State::cooling(0));
                    }
                } else if temp < self.set_point && self.mode == HeatSetting::Heating {
                    *ambient_change_count += 1;
                    if *ambient_change_count >= 5 {
                        return Transition(State::heating(0));
                    }
                }
                Super
            }
            Event::CoolingModeChanged(e) => {
                self.mode = e.clone();
                Handled
            }
            _ => Super,
        }
    }

    fn update_setpoint(&mut self, increase: bool) {
        self.set_point = if increase {
            self.set_point + 1
        } else {
            self.set_point - 1
        };

        self.hw.screen_write_setpoint(self.set_point);
    }

    #[action]
    fn idle_enter(&self) {
        self.relays_off();
        self.hw.report_idle();
    }

    #[state(entry_action = "cooling_enter", exit_action = "cooling_exit")]
    fn cooling(&mut self, equal_count: &mut u8, event: &Event) -> Response<State> {
        match event {
            Event::UpButtonPressed => {
                self.update_setpoint(true);
                if self.hw.read_temperature() >= self.set_point {
                    Transition(State::idle(0))
                } else {
                    Handled
                }
            }
            Event::DownButtonPressed => {
                self.update_setpoint(false);
                if self.hw.read_temperature() <= self.set_point {
                    Handled
                } else {
                    Transition(State::idle(0))
                }
            }
            Event::SecondPassed => {
                if self.hw.read_temperature() == self.set_point {
                    *equal_count += 1;
                    if *equal_count >= 5 {
                        return Transition(State::idle(0));
                    }
                }
                Handled
            }
            Event::CoolingModeChanged(e) => {
                self.mode = e.clone();
                if self.mode == HeatSetting::Heating {
                    Transition(State::idle(0))
                } else {
                    Handled
                }
            }
            _ => Super,
        }
    }

    #[action]
    fn cooling_enter(&self) {
        let hw = self.hw;
        hw.relay_on(Relay::Compressor);
        hw.relay_on(Relay::Fan);
        hw.report_cooling();
    }

    #[action]
    fn cooling_exit(&self) {
        self.relays_off();
    }

    #[state(entry_action = "heating_enter", exit_action = "heating_exit")]
    fn heating(&mut self, equal_count: &mut u8, event: &Event) -> Response<State> {
        match event {
            Event::UpButtonPressed => {
                self.update_setpoint(true);
                let temp = self.hw.read_temperature();
                if temp >= self.set_point {
                    Handled
                } else {
                    Transition(State::idle(0))
                }
            }
            Event::DownButtonPressed => {
                self.update_setpoint(false);
                let temp = self.hw.read_temperature();
                if temp <= self.set_point {
                    Transition(State::idle(0))
                } else {
                    Handled
                }
            }
            Event::SecondPassed => {
                let temp = self.hw.read_temperature();
                self.check_report_temp(temp);
                if temp == self.set_point {
                    *equal_count += 1;
                    if *equal_count >= 5 {
                        return Transition(State::idle(0));
                    }
                }
                Handled
            }
            Event::CoolingModeChanged(e) => {
                self.mode = e.clone();
                if self.mode == HeatSetting::Cooling {
                    Transition(State::idle(0))
                } else {
                    Handled
                }
            }
            _ => Super,
        }
    }

    #[action]
    fn heating_enter(&self) {
        self.relays_off();
        self.hw.report_heating();
    }

    #[action]
    fn heating_exit(&self) {}

    fn relays_off(&self) {
        self.hw.relay_off(Relay::Compressor);
        self.hw.relay_off(Relay::Heat);
        self.hw.relay_off(Relay::ReversingValve);
        self.hw.relay_off(Relay::Fan);
    }

    fn check_report_temp(&mut self, new_temp: u8) {
        if new_temp != self.last_reported_temp {
            self.hw.report_temperature(new_temp);
            self.last_reported_temp = new_temp;
        }
    }
}
