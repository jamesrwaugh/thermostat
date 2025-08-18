project(
    'thermostat',
    'cpp',
    'c',
    version: '0.5',
    default_options: [
        'cpp_std=c++20',
        'warning_level=1',
        'buildtype=minsize',
        'optimization=s',
    ],
)

add_project_arguments(
    [
        '-mmcu=atmega88p',
        '-DF_CPU=7372800',
        '-Wimplicit-fallthrough',
    ],
    language: ['cpp', 'c'],
)

add_project_arguments(['-Wno-c++20-compat'], language: ['cpp'])

assert(
    meson.is_cross_build(),
    'AVR projects can only be built in a cross build environment.',
)

toolkit = subproject('simavr-toolbox')
simavr_toolbox_dep = toolkit.get_variable('simavr_toolbox_dep')

subdir('src/avr-driver')
subdir('src/simulation')
subdir('src/app')