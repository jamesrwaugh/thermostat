use statig::{
    state_machine, IntoStateMachine,
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

#[derive(Copy, Clone, PartialEq, Debug)]
pub enum StateA {
    Idle,
    Active,
}

#[derive(Copy, Clone, PartialEq, Debug)]
pub enum Superstate {
    S,
}

pub struct BlinkyContext<'hw> {
    hw: &'hw dyn IHardware,
}

impl IntoStateMachine for Blinky {
    type State = StateA;

    type Superstate<'sub> = Superstate;

    type Event<'evt> = Event;

    type Context<'ctx> = BlinkyContext<'ctx>;

    const INITIAL: StateA = StateA::Idle;
}

impl Blinky {
    fn idle_enter(&mut self, context: &BlinkyContext) {}

    fn idle_exit(&mut self, context: &BlinkyContext) {}

    fn active_enter(&mut self, context: &BlinkyContext) {}

    fn active_exit(&mut self, context: &BlinkyContext) {}

    fn idle(event: &Event, context: &BlinkyContext) -> Response<StateA> {
        context.hw.read_temperature();
        match event {
            Event::ButtonPressed => Transition(StateA::Active),
            _ => Super,
        }
    }

    fn active(event: &Event, context: &BlinkyContext) -> Response<StateA> {
        context.hw.read_temperature();
        match event {
            Event::ButtonPressed => Transition(StateA::Active),
            _ => Super,
        }
    }
}

impl statig::blocking::State<Blinky> for StateA {
    fn call_handler(
        &mut self,
        shared_storage: &mut Blinky,
        event: &<Blinky as IntoStateMachine>::Event<'_>,
        context: &mut <Blinky as IntoStateMachine>::Context<'_>,
    ) -> Response<Self> {
        match self {
            StateA::Idle => Blinky::idle(event, &context),
            StateA::Active => Blinky::active(event, &context),
        }
    }

    fn call_entry_action(
        &mut self,
        shared_storage: &mut Blinky,
        context: &mut <Blinky as IntoStateMachine>::Context<'_>,
    ) {
        match self {
            StateA::Idle => Blinky::idle_enter(shared_storage, context),
            StateA::Active => Blinky::active_enter(shared_storage, context),
        }
    }

    fn call_exit_action(
        &mut self,
        shared_storage: &mut Blinky,
        context: &mut <Blinky as IntoStateMachine>::Context<'_>,
    ) {
        match self {
            StateA::Idle => Blinky::idle_exit(shared_storage, context),
            StateA::Active => Blinky::active_enter(shared_storage, context),
        }
    }
}

fn main() {
    // let mut state_machine = Blinky::default().state_machine();

    // state_machine.handle_with_context(&Event::TimerElapsed);
    // state_machine.handle(&Event::ButtonPressed);
}
