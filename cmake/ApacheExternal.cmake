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

#Apache Library Names
set(ARROW_STATIC_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}arrow")
set(ARROW_FULL_LIBRARY_NAME "${ARROW_STATIC_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(PARQUET_STATIC_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}parquet")
set(PARQUET_FULL_LIBRARY_NAME "${PARQUET_STATIC_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(DATASET_FULL_LIBRARY_NAME "libarrow_dataset.a")

set(APACHE_CMAKE_ARGS "-Wno-dev"
                      "-DCMAKE_INSTALL_PREFIX=${APACHE_INSTALL_DIR}"
                      "-DARROW_PARQUET=ON"                    #parquet
                      "-DPARQUET_BUILD_EXAMPLES=ON"
                      "-DARROW_BUILD_EXAMPLES=ON"
                      #"-DARROW_DATASET=ON"
                      "-DARROW_FILESYSTEM=ON"
                      "-DARROW_DEPENDENCY_SOURCE=BUNDLED"
                      #"-DARROW_IPC=ON"
                #       "-DARROW_WITH_BZ2=ON"                 #compression schemes
                #       "-DARROW_WITH_ZLIB=ON"
                #       "-DARROW_WITH_LZ4=ON"
                #       "-DARROW_WITH_SNAPPY=ON"
                #       "-DARROW_WITH_ZSTD=ON"
                      #"-DARROW_DEPENDENCY_SOURCE=BUNDLED"
                      #"-DARROW_S3=ON"                       #amazon s3
                      #"-DARROW_FILESYSTEM=ON"               #file system api
                      )


#Set Include Directory and Library
set(APACHE_INCLUDE_DIR "${APACHE_INSTALL_DIR}/include/")
set(ARROW_LIBRARY "${APACHE_INSTALL_DIR}/lib/${ARROW_FULL_LIBRARY_NAME}")
set(PARQUET_LIBRARY "${APACHE_INSTALL_DIR}/lib/${PARQUET_FULL_LIBRARY_NAME}")
set(DATASET_LIBRARY "${APACHE_INSTALL_DIR}/lib/${DATASET_FULL_LIBRARY_NAME}")

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