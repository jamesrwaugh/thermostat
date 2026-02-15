simulation_src = files(
  'main.cpp',
  'sim_sht4x.cpp',
  'sim_thermostat.cpp',
  'sim_tmp116.cpp',
  'ui.cpp',
)

toolkit = subproject('simavr-toolbox')
simavr_toolbox_dep = toolkit.get_variable('simavr_toolbox_dep')
ftxui_toolbox_dep = toolkit.get_variable('ftxui_toolbox_dep')

simavr_dep = dependency('simavr', native: true)
simavr_parts_dep = dependency('simavrparts', native: true)

ftxui_dep = dependency(
  'ftxui',
  native: true,
)

executable(
  'thermo-simulation',
  simulation_src,
  native: true,
  dependencies: [
    ft4222_dep,
    simavr_dep,
    ftxui_dep,
    simavr_toolbox_dep,
    simavr_parts_dep,
    ftxui_toolbox_dep,
    lodepng_dep,
    protos_dep_native,
  ],
  cpp_args: [
    '-std=c++20',
    #'-fsanitize=address,undefined',
  ],
  # link_args: ['-fsanitize=address,undefined'],
  include_directories: [],
  override_options: [
    'buildtype=debug',
    'optimization=0',
  ],
)

simulation__test_src = files(
  'main_test.cpp',
)

executable(
  'thermo-simulation-test',
  simulation__test_src,
  native: true,
  dependencies: [
    simavr_dep,
    ftxui_dep,
    simavr_toolbox_dep,
    simavr_parts_dep,
    ftxui_toolbox_dep,
    lodepng_dep,
    protos_dep_native,
  ],
  cpp_args: [
    '-std=c++20',
    #   '-fsanitize=address,undefined'
  ],
  # link_args: ['-fsanitize=address,undefined'],
  override_options: [
    'buildtype=debug',
    'optimization=0',
  ],
)
