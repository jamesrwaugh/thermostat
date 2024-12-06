main_src = files(
    'main.cpp',
    'rtc_interface.cpp',
    'screen_interface.cpp',
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
        meson.project_source_root() + '/src/thermo-app-avr',
        '--lang', 'c',
        '--parse-dependencies',
        '-o', '@OUTPUT0@',
    ],
    depends: [build_thermo_rs],
    build_always_stale: true,
)

drivers_src = files(
    'DS3231/src/driver_ds3231.c',
    'GU7000/Noritake_VFD_GU7000.cpp',
    'I2C/twi/twi_master.c',
    'TMP116/tmp116.cpp',
)

drivers_include = include_directories(
    'DS3231/src',
    'GU7000',
    'I2C/twi',
    'TMP116',
)

executable(
    'thermo-firmware',
    main_src,
    drivers_src,
    gen_thermo_h,
    pie: false,
    dependencies: [thermo_app_dep],
    include_directories: [drivers_include],
    cpp_args: ['-DETL_NO_STL'],
    link_args: [
        '-Wl,--gc-sections,--undefined=_mmcu,--section-start=.mmcu=0x910000',
        '-mmcu=atmega88p',
    ],
)