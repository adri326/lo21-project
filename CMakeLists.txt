cmake_minimum_required(VERSION 3.15)

project(lo21-project)

# option(FAITHFUL_IMPLEMENTATION "ON to enable faithful implementation (slower, default), OFF to disable it (faster)" ON)

if(FAITHFUL_IMPLEMENTATION)
  add_compile_definitions(FAITHFUL_IMPLEMENTATION)
endif()

set(SRC_FILES
    src/main.c
    src/rule.c
    src/knowledge.c
)

set(HEADER_FILES
    src/rule.h
    src/knowledge.h
)

add_executable(lo21-project ${SRC_FILES} ${HEADER_FILES})
