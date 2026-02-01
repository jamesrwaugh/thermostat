# comms

Minimal Rust firmware for **ESP32-C3**, built with [esp-hal](https://github.com/esp-rs/esp-hal). Implements a simple LED blink on GPIO 8 (change to GPIO 7 for ESP32-C3-DevKit-RUST-1).

## Prerequisites

- Rust (nightly): `rustup default nightly`
- ESP32-C3 target: `rustup target add riscv32imc-unknown-none-elf`
- [espup](https://github.com/esp-rs/espup) and toolchain (optional, for flashing): `espup install`

## Build

**Via Meson** (from repo root, in a cross-build):

```bash
meson compile -C builddir
# Output: builddir/src/comms/comms.elf
```

**Via Cargo** (from this directory):

```bash
cargo build --release
# Output: target/riscv32imc-unknown-none-elf/release/comms
```

## Flash and monitor

With [espflash](https://github.com/esp-rs/espflash) and board connected:

```bash
cargo run --release
# or: espflash flash --monitor target/riscv32imc-unknown-none-elf/release/comms
```
