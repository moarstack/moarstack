cmake_minimum_required(VERSION 3.0)
project(libmoar_presentation) 

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
add_library(moar_presentation SHARED ${SRC_FILES})
target_link_libraries(moar_presentation moar_shared)

#copy of config
if(DEFINED PRESENTATION_ENABLED)

else (DEFINED PRESENTATION_ENABLED)
	set(PRESENTATION_ENABLED 1)
endif(DEFINED PRESENTATION_ENABLED)

if(${PRESENTATION_ENABLED})
	set(CONFIG_TARGET_PATH ${CONFIG_LAYAERS_ENABLED_OUTPUT_PATH})
else(${PRESENTATION_ENABLED})
	set(CONFIG_TARGET_PATH ${CONFIG_LAYAERS_OUTPUT_PATH})
endif(${PRESENTATION_ENABLED})

file(GLOB LAYER_CONFIG
  "config/*.conf"
)
file(COPY ${LAYER_CONFIG} DESTINATION ${CONFIG_TARGET_PATH})