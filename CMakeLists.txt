cmake_minimum_required(VERSION 3.10)

# set the project name
project(succinct-col-compress   VERSION 2.0
                                DESCRIPTION "Splits a file into columns and uses succinct to compress each column"
                                LANGUAGES CXX)

#set executable location
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

#add module
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

#execute modules
include(LZ4External)
include(DSLibExternal)
include(SuccinctExternal)
#include(FST)
#include(FindSuccinct)

#add subdirectories
add_subdirectory(src)