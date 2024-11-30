use statig::{
    prelude::IntoStateMachineExt,
    state_machine,
    Response::{self, Super, Transition},
};

#[derive(Default)]
pub struct Blinky;

pub enum Event {
    TimerElapsed,
    ButtonPressed,
}

#[state_machine(initial = "State::led_on()")]
impl Blinky {
    #[state(superstate = "blinking")]
    fn led_on(event: &Event) -> Response<State> {
        match event {
            Event::TimerElapsed => Transition(State::led_off()),
            _ => Super,
        }
    }

    #[state(superstate = "blinking", entry_action = "enter_led_on")]
    fn led_off(event: &Event) -> Response<State> {
        match event {
            Event::TimerElapsed => Transition(State::led_on()),
            _ => Super,
        }
    }

    #[action]
    fn enter_led_on() {
        // Hey
    }

    #[superstate]
    fn blinking(event: &Event) -> Response<State> {
        match event {
            Event::ButtonPressed => Transition(State::not_blinking()),
            _ => Super,
        }
    }

    #[state]
    fn not_blinking(event: &Event) -> Response<State> {
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
