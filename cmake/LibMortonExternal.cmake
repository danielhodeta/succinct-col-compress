#This project defines:
# LIBMORTON_HOME -> the home directory of the external project
# LIBMORTON_SOURCE_DIR -> the directory where the git repo is cloned
# LIBMORTON_INCLUDE_DIR -> the include directory of the project


include(ExternalProject)

#set directories
set(LIBMORTON_PREFIX "${PROJECT_SOURCE_DIR}/external/libmorton-prefix")
set(LIBMORTON_HOME "${LIBMORTON_PREFIX}")
set(LIBMORTON_SOURCE_DIR "${LIBMORTON_PREFIX}/libmorton")
set(LIBMORTON_BUILD_DIR "${LIBMORTON_PREFIX}/build")

set(LIBMORTON_INCLUDE_DIR "${LIBMORTON_SOURCE_DIR}/libmorton/include")

ExternalProject_Add(libmorton
        GIT_REPOSITORY "https://github.com/Forceflow/libmorton.git"
        GIT_TAG "main"
        PREFIX "${LIBMORTON_PREFIX}"
        SOURCE_DIR "${LIBMORTON_SOURCE_DIR}"
        STAMP_DIR "${LIBMORTON_PREFIX}/stamp"
        BUILD_COMMAND ""
        INSTALL_COMMAND ""

        LOG_DOWNLOAD ON)

