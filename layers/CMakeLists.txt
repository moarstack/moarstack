cmake_minimum_required(VERSION 3.0)
project(moarlayers)

set(LIBRARY_OUTPUT_PATH ../lib)

add_subdirectory(interface)
add_subdirectory(channel)
add_subdirectory(routing)
add_subdirectory(presentation)
add_subdirectory(service)
add_subdirectory(shared)