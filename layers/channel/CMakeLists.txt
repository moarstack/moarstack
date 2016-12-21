cmake_minimum_required(VERSION 3.0)
project(libmoar_channel) 

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
add_library(moar_channel SHARED ${SRC_FILES})
target_link_libraries(moar_channel moar_shared)

#copy of config
if(DEFINED CHANNEL_ENABLED)

else (DEFINED CHANNEL_ENABLED)
	set(CHANNEL_ENABLED 1)
endif(DEFINED CHANNEL_ENABLED)

if(${CHANNEL_ENABLED})
	set(CONFIG_TARGET_PATH ${CONFIG_LAYERS_ENABLED_OUTPUT_PATH})
else(${CHANNEL_ENABLED})
	set(CONFIG_TARGET_PATH ${CONFIG_LAYERS_OUTPUT_PATH})
endif(${CHANNEL_ENABLED})

file(GLOB LAYER_CONFIG
  "config/*.conf"
)
file(COPY ${LAYER_CONFIG} DESTINATION ${CONFIG_TARGET_PATH})