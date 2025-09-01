src = files(
    'protos/ThermoStateData_bp.c',
    'states/cooling.cpp',
    'states/heating.cpp',
    'states/idle.cpp',
    'states/machine.cpp',
    'states/running_parent.cpp',
    'main.cpp',
)

etl_dep = dependency(
    'etl',
    method: 'cmake',
    modules: ['etl::etl'],
)

executable(
    'thermo-app',
    src,
    dependencies: [etl_dep, avr_driver_dep, bitproto_dep],
    cpp_args: ['-DETL_NO_STL', '-fno-threadsafe-statics'],
    link_args: [
        '-Wl,--gc-sections,--undefined=_mmcu,--section-start=.mmcu=0x910000',
        '-mmcu=atmega88p',
    ],
)
