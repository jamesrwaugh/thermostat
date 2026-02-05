proto_src = files(
  'HaCommandMailBox.cpp',
  'HomeAssistantSerial_bp.c',
  'ThermoSaveData_bp.c',
  'checksum.cpp',
)

proto_include = include_directories('.')

protos_lib = static_library(
  'thermo-protos',
  proto_src,
  pic: false,
  dependencies: [bitproto_dep],
  include_directories: [proto_include],
  link_args: [
    '-Wl,--gc-sections',
  ],
)

protos_dep = declare_dependency(
  link_with: protos_lib,
  include_directories: [proto_include],
  dependencies: [bitproto_dep],
)

protos_lib_native = static_library(
  'thermo-protos-native',
  proto_src,
  pic: true,
  native: true,
  dependencies: [bitproto_dep],
  include_directories: [proto_include],
  c_args: [
    '-funsigned-char',
    '-funsigned-bitfields',
    '-fshort-enums',
  ],
  override_options: [
    'buildtype=debug',
    'optimization=0',
  ],
)

protos_dep_native = declare_dependency(
  link_with: protos_lib_native,
  include_directories: [proto_include],
  dependencies: [bitproto_dep],
)
