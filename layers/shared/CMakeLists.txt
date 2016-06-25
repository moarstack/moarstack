cmake_minimum_required(VERSION 3.5)
project(libmoar_shared) 

set(LIBRARY_OUTPUT_PATH ../../lib)

file(GLOB SRC_FILES
    "src/*.c"
)

#add shared layers headers
include_directories(../shared/inc)

#add local headers
include_directories(inc)

#add library
add_library(moar_shared SHARED ${SRC_FILES})
