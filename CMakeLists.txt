cmake_minimum_required(VERSION 3.0)
project(moarstack)

set(LIBRARY_OUTPUT_PATH lib)
set(EXECUTABLE_OUTPUT_PATH  bin)

add_subdirectory(layers)
add_subdirectory(moard)
add_subdirectory(libmoar)

add_subdirectory(tools)
