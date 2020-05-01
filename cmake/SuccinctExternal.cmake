#This module defines:
# SUCCINCT_HOME -> home directory of succinct
# SUCCINCT_FULL_LIBRARY_NAME ->the name of the succinct library
# SUCCINCT_SOURCE_DIR -> the directory where the repo is cloned to
# SUCCINCT_BIN_DIR -> the directory where succinct is built

include(ExternalProject)

#Succinct Directories
set(SUCCINCT_PREFIX "${PROJECT_SOURCE_DIR}/external/succinct-cpp-prefix")
set(SUCCINCT_HOME "${SUCCINCT_PREFIX}")
set(SUCCINCT_SOURCE_DIR "${SUCCINCT_PREFIX}/succinct-cpp")
set(SUCCINCT_BIN_DIR "${SUCCINCT_PREFIX}/build")

#Succinct Library Name
set(SUCCINCT_STATIC_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}succinct")
set(SUCCINCT_FULL_LIBRARY_NAME "${SUCCINCT_STATIC_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")

#Set Include Directory and Library
set(SUCCINCT_INCLUDE_DIR "${SUCCINCT_SOURCE_DIR}/core/include")
set(SUCCINCT_LIBRARY "${SUCCINCT_BIN_DIR}/core/lib/${SUCCINCT_FULL_LIBRARY_NAME}")

ExternalProject_Add(succinct-cpp
        GIT_REPOSITORY "https://github.com/amplab/succinct-cpp.git"
        GIT_TAG "lambda"
        PREFIX "${SUCCINCT_PREFIX}"
        SOURCE_DIR "${SUCCINCT_SOURCE_DIR}"
        BINARY_DIR "${SUCCINCT_BIN_DIR}"
        STAMP_DIR "${SUCCINCT_PREFIX}/stamp"
        INSTALL_COMMAND ""

        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON)