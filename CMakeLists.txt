cmake_minimum_required(VERSION 3.0)
project(moarstack)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CONFIG_OUTPUT_PATH  ${CMAKE_CURRENT_BINARY_DIR}/config/moarstack)
set(CONFIG_LAYAERS_OUTPUT_PATH  ${CONFIG_OUTPUT_PATH}/layers_available)
set(CONFIG_LAYAERS_ENABLED_OUTPUT_PATH  ${CONFIG_OUTPUT_PATH}/layers_enabled)

#if(DEFINED CSTD)
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=" ${CSTD})
#else(DEFINED CSTD)
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
#endif(DEFINED CSTD)
#
if(DEFINED OPT)
    if(${OPT} EQUAL 0)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
    endif(${OPT} EQUAL 0)
    if(${OPT} EQUAL 1)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O")
    endif(${OPT} EQUAL 1)
    if(${OPT} EQUAL 2)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
    endif(${OPT} EQUAL 2)
    if(${OPT} EQUAL 3)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
    endif(${OPT} EQUAL 3)
    if(${OPT} EQUAL s)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
    endif(${OPT} EQUAL s)
else(DEFINED OPT)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
endif(DEFINED OPT)

if(${WARNINGS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
else(${WARNINGS})

endif(${WARNINGS})

if(${DEBUG})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")
    add_definitions(-DDEBUG)
    message("Debug enabled")
else(${DEBUG})

endif(${DEBUG})

#endif(DEFINED DEBUG)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fPIC")

add_subdirectory(layers)
add_subdirectory(moard)
add_subdirectory(libmoar)

add_subdirectory(tools)

file(GLOB LAYER_CONFIG
  "config/*.conf"
)
file(COPY ${LAYER_CONFIG} DESTINATION ${CONFIG_OUTPUT_PATH})
