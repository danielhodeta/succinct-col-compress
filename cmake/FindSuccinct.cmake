#This module defines:
# SUCCINCT_INCLUDE_DIR -> the directory that contains all include files
# SUCCINCT_LIBRARY -> the succinct library file

find_path(SUCCINCT_INCLUDE_DIR
        NAMES
        succinct_file.h
        HINTS
        ${SUCCINCT_SOURCE_DIR}/core/include
        PATH_SUFFIXES
        include)

find_library(SUCCINCT_LIBRARY
        NAMES
        ${SUCCINCT_FULL_LIBRARY_NAME}
        HINTS
        ${SUCCINCT_BIN_DIR}/core/
        PATH_SUFFIXES
        lib)

message(STATUS "INCLUDE directory: ${SUCCINCT_INCLUDE_DIR}")
message(STATUS "Succinct library: ${SUCCINCT_LIBRARY}")