cmake_minimum_required(VERSION 3.0)
project(libmoar_interface) 

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
add_library(moar_interface SHARED ${SRC_FILES})
target_link_libraries(moar_interface moar_shared)

#copy of config
if(DEFINED IFACE_MOCK_ENABLED)

else (DEFINED IFACE_MOCK_ENABLED)
	set(IFACE_MOCK_ENABLED 1)
endif(DEFINED IFACE_MOCK_ENABLED)

if(${IFACE_MOCK_ENABLED})
	set(CONFIG_TARGET_PATH ${CONFIG_LAYAERS_ENABLED_OUTPUT_PATH})
else(${IFACE_MOCK_ENABLED})
	set(CONFIG_TARGET_PATH ${CONFIG_LAYAERS_OUTPUT_PATH})
endif(${IFACE_MOCK_ENABLED})

file(GLOB LAYER_CONFIG
  "config/*.conf"
)
file(COPY ${LAYER_CONFIG} DESTINATION ${CONFIG_TARGET_PATH})