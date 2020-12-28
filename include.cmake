cmake_minimum_required(VERSION 3.15)

option(FAITHFUL_IMPLEMENTATION "ON to enable faithful implementation (slower, default), OFF to disable it (faster)" ON)
option(PRINT_ERRORS "ON to print out errors (default), OFF for silent mode" ON)

if(WIN32)
  set(DEFAULT_NO_COLOR OFF)
else()
  set(DEFAULT_NO_COLOR ON)
endif()

option(NO_COLOR "ON to enable colors (default on non-windows), OFF to disable colors (default on windows)" DEFAULT_NO_COLOR)

if(FAITHFUL_IMPLEMENTATION)
  add_compile_definitions(FAITHFUL_IMPLEMENTATION)
endif()

if(NO_COLOR)
  add_compile_definitions(NO_COLOR)
endif()

if(PRINT_ERRORS)
  add_compile_definitions(PRINT_ERRORS)
endif()

set(INFERENCE_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/src")

set(INFERENCE_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/src/inference.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/knowledge.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/rule.c"
)

set(INFERENCE_HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/src/colors.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/inference.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/knowledge.h"
    "${CMAKE_CURRENT_LIST_DIR}/src/rule.h"
)

add_library(inference ${INFERENCE_SOURCES} ${INFERENCE_HEADERS})
