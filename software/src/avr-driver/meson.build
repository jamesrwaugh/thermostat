main_src = files(
    'Serial/HardwareSerial.cpp',
    'Serial/HardwareSerial0.cpp',
    'Serial/Print.cpp',
    'driver.cpp',
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

etl_dep = dependency(
    'etl',
    method: 'cmake',
    modules: ['etl::etl'],
)

bindgen = find_program('bindgen')

gen_rust_bindings = custom_target(
    'gen-rs-bindings-bindings',
    output: ['_unused'],
    command: [
        bindgen,
        '--use-core', meson.project_source_root()
        + '/src/avr-driver/driver_rs_wrapper.hpp',
        '--merge-extern-blocks',
        '--rustified-enum', 'Relay',
        '--rustified-enum', 'Button',
        '--output', meson.project_source_root() + '/src/app/src/avr_driver_api.rs',
        # https://github.com/rust-lang/rust-bindgen/issues/2931
        '--no-layout-tests',
        # https://github.com/rust-lang/rust-bindgen/issues/3052#issuecomment-2531954670
        '--rust-target', '1.81.0',
        '--',
        '-I/usr/local/avr/avr/include',
        '-I/lib/gcc/x86_64-linux-gnu/14/include',
        '-x', 'c++',
    ],
    build_always_stale: true,
)

static_library(
    'thermo-avr-driver',
    main_src,
    drivers_src,
    gen_rust_bindings,
    pic: false,
    dependencies: [etl_dep],
    include_directories: [drivers_include],
    cpp_args: ['-DETL_NO_STL', '-fno-threadsafe-statics'],
    link_args: [
        '-Wl,--gc-sections,--undefined=_mmcu,--section-start=.mmcu=0x910000',
        '-mmcu=atmega88p',
    ],
)