cmake_minimum_required(VERSION 3.15)

project(lo21-project)

include(include.cmake)

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
