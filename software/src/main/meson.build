main_src = files(
    'main.cpp',
)

thermo_app_dep = declare_dependency(
    link_args: [
        '-L'
        + meson.project_source_root()
        + '/src/thermo-app-avr/target/avr-atmega88/release/',
        '-lthermoapp',
    ],
)

gen_thermo_h = custom_target(
    'gen-thermo-c-bindings',
    output: ['thermo.h'],
    command: [
        cbindgen,
        meson.project_source_root() + '/src/thermo-app',
        '--lang', 'c',
        '-o', '@OUTPUT0@',
    ],
    depends: [build_thermo_rs],
    build_always_stale: true,
)

executable(
    'thermo-firmware',
    main_src,
    gen_thermo_h,
    pie: false,
    dependencies: [thermo_app_dep],
    include_directories: [],
    cpp_args: [
        '-DETL_NO_STL',
        '-mmcu=atmega88p',
    ],
    link_args: [
        '-Wl,--gc-sections,--undefined=_mmcu,--section-start=.mmcu=0x910000',
        '-mmcu=atmega88p',
    ],
)