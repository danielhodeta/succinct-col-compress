#This module defines:
# SURF_HOME -> home directory of 
# SURF_FULL_LIBRARY_NAME ->the name of the SuRF library
# SURF_SOURCE_DIR -> the directory where the repo is cloned to
# SURF_BIN_DIR -> the directory where SuRF is built

include(ExternalProject)

#SuRF Directories
set(SURF_PREFIX "${PROJECT_SOURCE_DIR}/external/SuRF-prefix")
set(SURF_HOME "${SURF_PREFIX}")
set(SURF_SOURCE_DIR "${SURF_PREFIX}/SuRF")
set(SURF_BIN_DIR "${SURF_PREFIX}/build")

#SuRF Library Name
set(SURF_STATIC_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}SuRF")
set(SURF_FULL_LIBRARY_NAME "${SURF_STATIC_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")

#Set Include Directory and Library
set(SURF_INCLUDE_DIR "${SURF_SOURCE_DIR}/core/include")
#set(SURF_LIBRARY "${SURF_BIN_DIR}/core/lib/${SURF_FULL_LIBRARY_NAME}")

ExternalProject_Add(SuRF-cpp
        GIT_REPOSITORY "https://github.com/efficient/SuRF.git"
        PREFIX "${SURF_PREFIX}"
        SOURCE_DIR "${SURF_SOURCE_DIR}"
        BINARY_DIR "${SURF_BIN_DIR}"
        STAMP_DIR "${SURF_PREFIX}/stamp"
        INSTALL_COMMAND ""

        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON)