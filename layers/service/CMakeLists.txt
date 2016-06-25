cmake_minimum_required(VERSION 3.0)
project(libmoar_service) 

set(LIBRARY_OUTPUT_PATH ../../lib)

file(GLOB SRC_FILES
    "src/*.c"
)

#add shared layers headers
include_directories(../shared/inc)

#add local headers
include_directories(inc)

#add library
add_library(moar_service SHARED ${SRC_FILES})
target_link_libraries(moar_service moar_shared)