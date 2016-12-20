cmake_minimum_required(VERSION 3.0)
project(libmoar_service) 

#set(LIBRARY_OUTPUT_PATH ../../lib)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g3 -fPIC")

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

#copy of config
if(DEFINED SERIVCE_ENABLED)

else (DEFINED SERIVCE_ENABLED)
	set(SERIVCE_ENABLED 1)
endif(DEFINED SERIVCE_ENABLED)

if(${SERIVCE_ENABLED})
	set(CONFIG_TARGET_PATH ${CONFIG_LAYAERS_ENABLED_OUTPUT_PATH})
else(${SERIVCE_ENABLED})
	set(CONFIG_TARGET_PATH ${CONFIG_LAYAERS_OUTPUT_PATH})
endif(${SERIVCE_ENABLED})

file(GLOB LAYER_CONFIG
  "config/*.conf"
)
file(COPY ${LAYER_CONFIG} DESTINATION ${CONFIG_TARGET_PATH})