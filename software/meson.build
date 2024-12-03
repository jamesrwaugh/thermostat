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

cbindgen = find_program('cbindgen')
cargo = find_program('cargo')

subdir('src/thermo-app-avr')
subdir('src/main')
subdir('src/simulation')