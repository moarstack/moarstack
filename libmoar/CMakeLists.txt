cmake_minimum_required(VERSION 3.0)
project(libmoar) 

set(LIBRARY_OUTPUT_PATH ../lib)

file(GLOB SRC_FILES
    "src/*.c"
)

#add shared layers headers
include_directories(../layers/shared/inc)

#add local headers
include_directories(inc)

#add library
add_library(moar SHARED ${SRC_FILES})
target_link_libraries(moar moar_shared)