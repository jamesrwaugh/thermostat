proto_src = files(
    'ThermoSaveData_bp.c',
)

proto_include = include_directories('.')

protos_lib = static_library(
    'thermo-protos',
    proto_src,
    pic: false,
    dependencies: [bitproto_dep],
    include_directories: [proto_include],
    c_args: [
        '-funsigned-char',
        '-funsigned-bitfields',
        '-fshort-enums',
        '-mcall-prologues',
    ],
)

protos_dep = declare_dependency(
    link_with: protos_lib,
    include_directories: [proto_include],
    dependencies: [bitproto_dep],
)