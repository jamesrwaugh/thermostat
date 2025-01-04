use crate::{
    avr_driver_api::{
        DriverDisplayIsCooling, DriverDisplayIsHeating, DriverDisplayIsIdle, DriverDisplaySetPoint,
        DriverDisplayTemp, DriverReadTemp, DriverRelayOn, Relay, Relay_Compressor, Relay_Fan,
        Relay_Heat, Relay_ReversingValve,
    },
    hardware::{self, IHardware},
};

#[derive(Default)]
pub struct AvrHardware;

impl AvrHardware {
    fn hardware_2_avr_relay(&self, r: hardware::Relay) -> Relay {
        match r {
            crate::hardware::Relay::Fan => Relay_Fan,
            crate::hardware::Relay::Compressor => Relay_Compressor,
            crate::hardware::Relay::Heat => Relay_Heat,
            crate::hardware::Relay::ReversingValve => Relay_ReversingValve,
        }
    }
}

impl IHardware for AvrHardware {
    fn screen_write_temperature(&self, temp: u8) {
        unsafe {
            DriverDisplayTemp(temp);
        }
    }

    fn screen_write_setpoint(&self, set_point: u8) {
        unsafe {
            DriverDisplaySetPoint(set_point);
        }
    }

    fn relay_on(&self, r: crate::hardware::Relay) {
        unsafe {
            DriverRelayOn(self.hardware_2_avr_relay(r));
        }
    }

    fn relay_off(&self, r: crate::hardware::Relay) {
        unsafe {
            DriverRelayOn(self.hardware_2_avr_relay(r));
        }
    }

    fn read_temperature(&self) -> u8 {
        unsafe {
            return DriverReadTemp();
        }
    }

    fn report_idle(&self) {
        unsafe {
            DriverDisplayIsIdle();
        }
    }

    fn report_cooling(&self) {
        unsafe {
            DriverDisplayIsCooling();
        }
    }

    fn report_heating(&self) {
        unsafe {
            DriverDisplayIsHeating();
        }
    }

    fn report_temperature(&self, new_temp: u8) {
        unsafe {
            DriverDisplayTemp(new_temp);
        }
    }
}
