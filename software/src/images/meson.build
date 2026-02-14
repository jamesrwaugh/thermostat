src = files(
    'images.c',
)

images_include = include_directories('.')

images_lib = static_library(
    'thermo-images',
    src,
    pic: false,
    c_args: [
        '-fshort-enums',
    ],
    link_args: [
        '-Wl,--gc-sections',
    ],
)

images_dep = declare_dependency(
    link_with: images_lib,
    include_directories: [images_include],
)
