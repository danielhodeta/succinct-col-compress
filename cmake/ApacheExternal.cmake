#This module defines:
# APACHE_HOME -> home directory of apache
# APACHE_FULL_LIBRARY_NAME ->the name of the apache library
# APACHE_SOURCE_DIR -> the directory where the repo is cloned to
# APACHE_BIN_DIR -> the directory where apache is built

include(ExternalProject)

#Apache Directories
set(APACHE_PREFIX "${PROJECT_SOURCE_DIR}/external/apache-cpp-prefix")
set(APACHE_HOME "${APACHE_PREFIX}")
set(APACHE_SOURCE_DIR "${APACHE_PREFIX}/apache-cpp")
set(APACHE_BIN_DIR "${APACHE_PREFIX}/build")
set(APACHE_INSTALL_DIR "${APACHE_PREFIX}/install")

#Apache Library Name
set(APACHE_STATIC_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}apache")
set(APACHE_FULL_LIBRARY_NAME "${APACHE_STATIC_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")

set(APACHE_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${APACHE_INSTALL_DIR}")

#Set Include Directory and Library
set(APACHE_INCLUDE_DIR "${APACHE_INSTALL_DIR}/include/arrow/")
set(APACHE_LIBRARY "${APACHE_INSTALL_DIR}/lib/${APACHE_FULL_LIBRARY_NAME}")

ExternalProject_Add(apache-cpp
        GIT_REPOSITORY "https://github.com/apache/arrow.git"
        PREFIX "${APACHE_PREFIX}"

        CMAKE_ARGS ${APACHE_CMAKE_ARGS}

        SOURCE_DIR "${APACHE_SOURCE_DIR}"
        SOURCE_SUBDIR "cpp"
        BINARY_DIR "${APACHE_BIN_DIR}"
        STAMP_DIR "${APACHE_PREFIX}/stamp"
        
        #INSTALL_COMMAND ""

        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON)