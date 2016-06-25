cmake_minimum_required(VERSION 3.5)
project(moard) 

set(LIBRARY_OUTPUT_PATH ../lib)
set(EXECUTABLE_OUTPUT_PATH  ../bin)

file(GLOB SRC_FILES
    "src/*.c"
)

#add shared layers headers
include_directories(../layers/shared/inc)

#add local headers
include_directories(inc)

#add library
add_executable(moard ${SRC_FILES})
target_link_libraries(moard moar_shared)