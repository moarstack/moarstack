cmake_minimum_required(VERSION 3.0)
project(moard) 

#set(LIBRARY_OUTPUT_PATH ../lib)
#set(EXECUTABLE_OUTPUT_PATH  ../bin)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g3 -fPIC")

file(GLOB SRC_FILES
    "src/*.c"
)

if(${USE_CLION_PATH})
    message("Used clion path")
    add_definitions(-DUSE_CLION_PATH)
endif(${USE_CLION_PATH})

if(${USE_NEWLM})
    if(${DEBUG})
        message("DEBUG and USE_NEWLM switchs not compatible, ignoring USE_NEWLM")
    else(${DEBUG})
        message("Using NEWLM library loading")
        add_definitions(-DUSE_NEWLM)
    endif(${DEBUG})
endif(${USE_NEWLM})


#add shared layers headers
include_directories(../layers/shared/inc)

#add local headers
include_directories(inc)

#add library
add_executable(moard ${SRC_FILES})
target_link_libraries(moard moar_shared)
target_link_libraries(moard dl)
target_link_libraries(moard pthread)