cmake_minimum_required(VERSION 3.0)
project(moarstack)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_CURRENT_BINARY_DIR}/bin)

if(DEFINED CSTD)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=" ${CSTD})
else(DEFINED CSTD)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif(DEFINED CSTD)

if(DEFINED OPT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O" ${OPT})
else(DEFINED OPT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
endif(DEFINED OPT)

#
#if(DEFINED DEBUG)
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g" ${DEBUG})
#    add_definitions(-DDEBUG)
#else(DEFINED DEBUG)
#
#endif(DEFINED DEBUG)

add_subdirectory(layers)
add_subdirectory(moard)
add_subdirectory(libmoar)

add_subdirectory(tools)
