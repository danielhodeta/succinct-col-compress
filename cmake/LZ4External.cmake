#This module defines:
# LZ4_HOME -> home directory of LZ4
# LZ4_FULL_LIBRARY_NAME ->the name of the LZ4 library
# LZ4_SOURCE_DIR -> the directory where the repo is cloned to
# LZ4_INCLUDE_DIR -> path to the header files
# LZ4_LIBRARY -> primary lz4 library

include(ExternalProject)

#LZ4 Directories
set(LZ4_PREFIX "${PROJECT_SOURCE_DIR}/external/lz4-prefix")
set(LZ4_HOME "${LZ4_PREFIX}")
set(LZ4_SOURCE_DIR "${LZ4_PREFIX}/lz4")

#LZ4 Library Name
set(LZ4_STATIC_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}lz4")
set(LZ4_FULL_LIBRARY_NAME "${LZ4_STATIC_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")

#Set Include Directory and Library
set(LZ4_INCLUDE_DIR "${LZ4_SOURCE_DIR}/lib/")
set(LZ4_LIBRARY "${LZ4_SOURCE_DIR}/lib/${LZ4_FULL_LIBRARY_NAME}")

ExternalProject_Add(lz4
        GIT_REPOSITORY "https://github.com/lz4/lz4.git"
        PREFIX "${LZ4_PREFIX}"
        SOURCE_DIR "${LZ4_SOURCE_DIR}"
        STAMP_DIR "${LZ4_PREFIX}/stamp"
        BINARY_DIR "${LZ4_SOURCE_DIR}"
        CONFIGURE_COMMAND ""
        INSTALL_COMMAND ""


        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON)