cmake_minimum_required(VERSION 3.15)

project(lo21-project)

set(SRC_FILES
  src/main.c
)

set(HEADER_FILES)

add_executable(lo21-project ${SRC_FILES} ${HEADER_FILES})
