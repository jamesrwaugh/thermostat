# Build the comms Rust project for ESP32-C3 (riscv32imc-unknown-none-elf).
# Requires: rustup target add riscv32imc-unknown-none-elf, and nightly for build-std.

comms_src_dir = meson.current_source_dir()

custom_target(
  'comms.elf',
  output: 'comms.elf',
  input: [
    join_paths(comms_src_dir, 'Cargo.toml'),
    join_paths(comms_src_dir, 'src', 'main.rs'),
  ],
  command: [
    find_program('sh'),
    '-c', 'cd "@0@" && cargo build --release'.format(
      comms_src_dir,
    ),
  ],
  build_by_default: true,
)