cmake_minimum_required(VERSION 3.0)
project(libmoar_shared) 

set(LIBRARY_OUTPUT_PATH ../../lib)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g3 -fPIC")

file(GLOB SRC_FILES
    "src/*.c"
)

#add shared layers headers
#include_directories(../shared/inc)

#add local headers
include_directories(inc)

#add library
add_library(moar_shared SHARED ${SRC_FILES})
target_link_libraries(moar_shared rt)