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

executable(
    'thermo-firmware',
    main_src,
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