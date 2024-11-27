build_thermo_rs = custom_target(
    'build-thermo-rs',
    command: [
        cargo,
        'build',
        '--manifest-path', meson.project_source_root() + '/src/thermo-app-avr/Cargo.toml',
        '--release',
    ],
    output: ['_dummy_out'],
    build_always_stale: true,
)