main_src = files(
    'Serial/HardwareSerial.cpp',
    'Serial/HardwareSerial0.cpp',
    'Serial/Print.cpp',
    'driver.cpp',
    'c_api/driver_rs_wrapper.cpp',
)

drivers_src = files(
    'DS1307/src/driver_ds1307.c',
    'GU7000/Noritake_VFD_GU7000.cpp',
    'I2C/twi/twi_master.c',
    'TMP116/tmp116.cpp',
)

drivers_include = include_directories(
    'DS1307/src',
    'GU7000',
    'I2C/twi',
    'TMP116',
    'Serial',
)

etl_dep = dependency(
    'etl',
    method: 'cmake',
    modules: ['etl::etl'],
)

arv_driver = static_library(
    'thermo-avr-driver',
    main_src,
    drivers_src,
    pic: false,
    dependencies: [etl_dep],
    include_directories: [drivers_include],
    cpp_args: ['-DETL_NO_STL', '-fno-threadsafe-statics'],
    link_args: [
        '-Wl,--gc-sections,--undefined=_mmcu,--section-start=.mmcu=0x910000',
        '-mmcu=atmega88p',
    ],
)

avr_driver_dep = declare_dependency(
    link_with: arv_driver,
    include_directories: [
        'c_api',
    ],
)