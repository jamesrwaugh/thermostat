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
    '-mmcu=atmega168p',
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

lodepng_dep = declare_dependency(
  sources: ['submodules/lodepng/lodepng.cpp'],
  include_directories: ['submodules/lodepng'],
)

ft4222_dep = dependency(
  'ft4222',
  native: true,
  required: false,
)

subdir('src/protos')
subdir('src/avr-driver')
subdir('src/simulation')
subdir('src/app')
subdir('src/ftdi-i2c-test')