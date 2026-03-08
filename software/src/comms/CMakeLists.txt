# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Pick up protos component from sibling src/protos (same sources as Meson build).
set(EXTRA_COMPONENT_DIRS "${CMAKE_SOURCE_DIR}/../protos")

# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)
project(hello_world)