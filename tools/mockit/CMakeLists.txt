cmake_minimum_required(VERSION 3.0)
project(moarMockIT) 

set(LIBRARY_OUTPUT_PATH ../../lib)
set(EXECUTABLE_OUTPUT_PATH  ../../bin)
set(CONFIG_OUTPUT_PATH  ../../bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g3 -fPIC")
set(CONFIG_FILE_PATH config)
set(CONFIG_FILE_NAME config.txt)

file(GLOB SRC_FILES
    "src/*.c"
)

include_directories(inc)

#add library
add_executable(moarMockIT ${SRC_FILES})
target_link_libraries(moarMockIT m)
file(COPY ${CONFIG_FILE_PATH}/${CONFIG_FILE_NAME} DESTINATION  ${CONFIG_OUTPUT_PATH})