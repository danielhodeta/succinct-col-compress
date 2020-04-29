#This project defines:
# DSLIB_HOME -> the home directory of the external project
# DSLIB_SOURCE_DIR -> the directory where the git repo is cloned
# DSLIB_INCLUDE_DIR -> the include directory of the project


include(ExternalProject)

#set directories
set(DSLIB_PREFIX "${PROJECT_SOURCE_DIR}/external/ds-lib-prefix")
set(DSLIB_HOME "${DSLIB_PREFIX}")
set(DSLIB_SOURCE_DIR "${DSLIB_PREFIX}/ds-lib")
set(DSLIB_BUILD_DIR "${DSLIB_PREFIX}/build")

set(DSLIB_INCLUDE_DIR "${DSLIB_SOURCE_DIR}/include")

ExternalProject_Add(ds-lib
        GIT_REPOSITORY "https://github.com/anuragkh/ds-lib.git"
        PREFIX "${DSLIB_PREFIX}"
        SOURCE_DIR "${DSLIB_SOURCE_DIR}"
        STAMP_DIR "${DSLIB_PREFIX}/stamp"
        BUILD_COMMAND ""
        INSTALL_COMMAND ""

        LOG_DOWNLOAD ON)

