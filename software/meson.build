project('cpp_rust_integration', ['cpp', 'rust'])

# Compiler options
cpp_options = ['-std=c++20']

# Subdirectories
subdir('src/abstract')
subdir('src/real')

# Main executable
executable(
    'main',
    'main.cpp',
    dependencies: [cpp_lib_dep, rust_dep],
    cpp_args: cpp_options,
)