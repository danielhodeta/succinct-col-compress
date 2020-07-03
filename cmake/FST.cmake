#This module defines:
# FST_HOME -> home directory of 
# FST_FULL_LIBRARY_NAME ->the name of the FST library
# FST_SOURCE_DIR -> the directory where the repo is cloned to
# FST_BIN_DIR -> the directory where FST is built

include(ExternalProject)

#FST Directories
set(FST_PREFIX "${PROJECT_SOURCE_DIR}/fst")
set(FST_HOME "${FST_PREFIX}")
set(FST_SOURCE_DIR "${FST_PREFIX}/fast-succinct-trie")
set(FST_BIN_DIR "${FST_PREFIX}/build")

#FST Library Name
set(FST_STATIC_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}FST")
set(FST_FULL_LIBRARY_NAME "${FST_STATIC_LIB_NAME}.so")

#Set Include Directory and Library
set(FST_INCLUDE_DIR "${FST_SOURCE_DIR}/fst/include")
set(FST_LIBRARY "${FST_BIN_DIR}/fst/src/${FST_FULL_LIBRARY_NAME}")

ExternalProject_Add(fast-succinct-trie
        GIT_REPOSITORY "https://github.com/efficient/fast-succinct-trie.git"
        DOWNLOAD_COMMAND ""
        UPDATE_COMMAND ""
        #CONFIGURE_COMMAND ""
        #BUILD_COMMAND ""
        BUILD_ALWAYS ON
        PREFIX "${FST_PREFIX}"
        SOURCE_DIR "${FST_SOURCE_DIR}"
        BINARY_DIR "${FST_BIN_DIR}"
        STAMP_DIR "${FST_PREFIX}/stamp"
        INSTALL_COMMAND ""

        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON)