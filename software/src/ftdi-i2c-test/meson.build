src = files(
  'GU7000/Noritake_VFD_GU7000.cpp',
  'ftdi.cpp',
  'main.cpp',
)

ft4222_dep = dependency(
  'ft4222',
  native: true,
  required: false,
)

if ft4222_dep.found()
  executable(
    'ftdi-i2c',
    src,
    native: true,
    dependencies: [ft4222_dep],
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
endif