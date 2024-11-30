use statig::{
    prelude::IntoStateMachineExt,
    state_machine,
    Response::{self, Super, Transition},
};

use crate::hardware::IHardware;

#[derive(Default)]
pub struct Blinky {}

pub enum Event {
    SecondPassed,
    TimerElapsed,
    ButtonPressed,
}

struct BlinkyContext<'hw> {
    hw: &'hw dyn IHardware,
}

#[state_machine(initial = "State::idle()")]
impl<'hw> Blinky {
    #[action]
    fn enter_idle() {
        // Hey
    }

    #[state(entry_action = "enter_idle")]
    fn idle(&self, context: &BlinkyContext, event: &Event) -> Response<State> {
        match event {
            Event::ButtonPressed => Transition(State::led_on()),
            _ => Super,
        }
    }

    #[state]
    fn active(event: &Event) -> Response<State> {
        match event {
            Event::ButtonPressed => Transition(State::led_on()),
            _ => Super,
        }
    }
}

fn main() {
    let mut state_machine = Blinky::default().state_machine();

    state_machine.handle(&Event::TimerElapsed);
    state_machine.handle(&Event::ButtonPressed);
}
