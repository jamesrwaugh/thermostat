simulation_src = files(
    'ftxui_simulated_avr.cpp',
    'main.cpp',
    'sim_gu700.cpp',
    'sim_i2c_smarter_base.cpp',
    'sim_thermostat.cpp',
    'sim_tmp116.cpp',
    'ui.cpp',
)

toolkit = subproject('simavr-toolbox')
simavr_toolbox_dep = toolkit.get_variable('simavr_toolbox_dep')

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
    ],
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