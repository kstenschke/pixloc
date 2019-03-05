cmake_minimum_required(VERSION 3.9)
project(pixloc
        VERSION 0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/)

include (${CMAKE_ROOT}/Modules/FindX11.cmake)
message("X11_FOUND: ${X11_FOUND}")

add_definitions(-DCMAKE_HAS_X)
#include_directories(${X11_INCLUDE_DIR})

include_directories(
        "${CMAKE_SOURCE_DIR}/src"
        ${X11_INCLUDE_DIR}
)

add_executable(pixloc
        src/pixloc/main.cc
        src/pixloc/cli_options.cc
        src/pixloc/helper/helper_strings.cc
        src/pixloc/models/color_matcher_abstract.cc
        src/pixloc/models/color_matcher_precise.cc
        src/pixloc/models/color_matcher_range.cc
        src/pixloc/models/pixel_scanner.cc)

target_link_libraries(pixloc ${X11_LIBRARIES})
