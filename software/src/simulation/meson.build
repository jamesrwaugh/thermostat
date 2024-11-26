simulation_src = files(
    'main.cpp',
)

simavr_dep = dependency('simavr', native: true)

ftxui_dep = dependency(
    'ftxui',
    native: true,
)

executable(
    'thermo-simulation',
    simulation_src,
    native: true,
    dependencies: [simavr_dep, ftxui_dep],
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