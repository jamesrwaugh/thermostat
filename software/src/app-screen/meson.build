src = files(
    '../app/protos/ThermoSaveData_bp.c',
    'main.cpp',
    'twi_master.c',
)

executable(
    'thermo-screen-app',
    src,
    dependencies: [
        avr_driver_dep,
        bitproto_dep,
    ],
    include_directories: ['../app/protos'],
    cpp_args: [
        '-DETL_NO_STL',
        '-fno-threadsafe-statics',
        '-funsigned-char',
        '-funsigned-bitfields',
        '-fshort-enums',
        '-mcall-prologues',
    ],
    link_args: [
        '-Wl,--gc-sections,--undefined=_mmcu,--section-start=.mmcu=0x910000',
        '-mmcu=atmega324p',
    ],
)