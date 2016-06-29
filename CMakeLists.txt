cmake_minimum_required(VERSION 3.0)
project(moarstack)

set(LIBRARY_OUTPUT_PATH lib)
set(EXECUTABLE_OUTPUT_PATH  bin)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

add_subdirectory(layers)
add_subdirectory(moard)
add_subdirectory(libmoar)

add_subdirectory(tools)
