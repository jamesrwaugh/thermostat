src = files(
  'GU7000/Noritake_VFD_GU7000.cpp',
  'ftdi.cpp',
  'main.cpp',
  'renderer.cpp',
  'scroll_manager.cpp',
  'scroller.cpp',
)

if ft4222_dep.found()
  executable(
    'ftdi-i2c',
    src,
    native: true,
    dependencies: [
      ft4222_dep,
      etl_dep,
      protos_dep_native,
    ],
    cpp_args: [
      '-DETL_NO_STL',
      '-fno-threadsafe-statics',
      '-funsigned-char',
      '-funsigned-bitfields',
      '-fshort-enums',
    ],
    # link_args: ['-fsanitize=address,undefined'],
    include_directories: [],
    override_options: [
      'buildtype=debug',
      'optimization=0',
      'cpp_std=c++20',
    ],
  )
endif
