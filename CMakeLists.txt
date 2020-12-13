cmake_minimum_required(VERSION 3.15)

project(lo21-project)

option(FAITHFUL_IMPLEMENTATION "ON to enable faithful implementation (slower, default), OFF to disable it (faster)" ON)

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

set(SRC_FILES
    src/main.c
    src/rule.c
    src/knowledge.c
    src/inference.c
)

set(HEADER_FILES
    src/rule.h
    src/knowledge.h
    src/inference.h
)

add_executable(lo21-project ${SRC_FILES} ${HEADER_FILES})
