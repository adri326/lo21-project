cmake_minimum_required(VERSION 3.15)

project(lo21-project-ext)

option(GENERATE_OPPOSITE "ON to enable opposite rule generation, OFF to disable" ON)

if(GENERATE_OPPOSITE)
  add_compile_definitions(GENERATE_OPPOSITE)
endif()

include(../include.cmake)
include(lists/include.cmake)

set(SRC_FILES
    src/main.c
    src/parse.c
)

set(HEADER_FILES
    src/parse.h
)

add_executable(lo21-project-ext ${SRC_FILES} ${HEADER_FILES})

# Add lists.c
target_link_libraries(lo21-project-ext PUBLIC lists-c)
target_include_directories(lo21-project-ext PUBLIC ${LISTS_C_INCLUDE_DIR})

# Add the inference engine
target_link_libraries(lo21-project-ext PUBLIC inference)
target_include_directories(lo21-project-ext PUBLIC ${INFERENCE_INCLUDE_DIR})
