src = files(
  'render/renderer.cpp',
  'render/scroll_manager.cpp',
  'render/scroller.cpp',
  'states/coolable_parent.cpp',
  'states/cooling.cpp',
  'states/heating.cpp',
  'states/idle.cpp',
  'states/machine.cpp',
  'states/program_autotime.cpp',
  'states/program_screen.cpp',
  'states/program_types.cpp',
  'states/started.cpp',
  'main.cpp',
  'safe_thermo_safe.cpp',
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
  dependencies: [etl_dep, avr_driver_dep, protos_dep],
  cpp_args: [
    '-DETL_NO_STL',
    '-fno-threadsafe-statics',
    '-funsigned-char',
    '-funsigned-bitfields',
    '-fshort-enums',
  ],
  include_directories: sim_avr_avr_inc,
  link_args: [
    '-Wl,--gc-sections,--undefined=_mmcu,--section-start=.mmcu=0x910000',
    '-mmcu=atmega168p',
  ],
)

executable(
  'thermo-app-test',
  'main_test.cpp',
  dependencies: [etl_dep, avr_driver_dep, protos_dep],
  cpp_args: [
    '-DETL_NO_STL',
    '-fno-threadsafe-statics',
    '-funsigned-char',
    '-funsigned-bitfields',
    '-fshort-enums',
  ],
  include_directories: sim_avr_avr_inc,
  link_args: [
    '-Wl,--gc-sections,--undefined=_mmcu,--section-start=.mmcu=0x910000',
    '-mmcu=atmega168p',
  ],
)
