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

bitproto_dep = declare_dependency(
    sources: ['submodules/bitproto/lib/c/bitproto.c'],
    include_directories: ['submodules/bitproto/lib/c/'],
)

bmodbus_dep = declare_dependency(
    sources: ['submodules/bmodbus/bmodbus.c'],
    include_directories: ['submodules/bmodbus/'],
    compile_args: ['-DBMODBUS_NO_MASTER'],
)

subdir('src/avr-driver')
subdir('src/simulation')
subdir('src/app')
