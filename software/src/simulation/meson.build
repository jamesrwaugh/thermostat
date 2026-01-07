simulation_src = files(
    'main.cpp',
    'sim_gu700.cpp',
    'sim_i2c_smarter_base.cpp',
    'sim_thermostat.cpp',
    'sim_tmp116.cpp',
    'sim_gu7000_real.cpp',
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
        simavr_dep,
        ftxui_dep,
        simavr_toolbox_dep,
        simavr_parts_dep,
        ftxui_toolbox_dep,
    ],
    cpp_args: ['-std=c++20'],
    include_directories: [],
    override_options: [
        'buildtype=debug',
        'optimization=0',
    ],
    # cpp_args: [
    #   '-fsanitize=address,undefined',
    # ],
    # link_args: ['-fsanitize=address,undefined'],
)
