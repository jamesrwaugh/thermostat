src = files(
    'protos/ThermoSaveData_bp.c',
    'states/coolable_parent.cpp',
    'states/cooling.cpp',
    'states/heating.cpp',
    'states/idle.cpp',
    'states/machine.cpp',
    'states/program.cpp',
    'main.cpp',
    'sim.c',
)

etl_dep = dependency(
    'etl',
    method: 'cmake',
    modules: ['etl::etl'],
)

sim_avr_avr_inc = ['/usr/local/include/simavr/avr']

executable(
    'thermo-app',
    src,
    dependencies: [
        etl_dep,
        avr_driver_dep,
        bitproto_dep,
        bmodbus_dep,
    ],
    cpp_args: [
        '-DETL_NO_STL',
        '-fno-threadsafe-statics',
        '-funsigned-char',
        '-funsigned-bitfields',
        '-fshort-enums',
        '-mcall-prologues',
    ],
    include_directories: sim_avr_avr_inc,
    link_args: [
        '-Wl,--gc-sections,--undefined=_mmcu,--section-start=.mmcu=0x910000',
        '-mmcu=atmega88p',
    ],
)