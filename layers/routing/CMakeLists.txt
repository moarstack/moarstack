cmake_minimum_required(VERSION 3.0)
project(libmoar_routing) 

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
add_library(moar_routing SHARED ${SRC_FILES})
target_link_libraries(moar_routing moar_shared)

#copy of config
if(DEFINED ROUTING_ENABLED)

else (DEFINED ROUTING_ENABLED)
	set(ROUTING_ENABLED 1)
endif(DEFINED ROUTING_ENABLED)

if(${ROUTING_ENABLED})
	set(CONFIG_TARGET_PATH ${CONFIG_LAYERS_ENABLED_OUTPUT_PATH})
else(${ROUTING_ENABLED})
	set(CONFIG_TARGET_PATH ${CONFIG_LAYERS_OUTPUT_PATH})
endif(${ROUTING_ENABLED})

file(GLOB LAYER_CONFIG
  "config/*.conf"
)
file(COPY ${LAYER_CONFIG} DESTINATION ${CONFIG_TARGET_PATH})