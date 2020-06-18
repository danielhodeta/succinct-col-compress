cmake_minimum_required(VERSION 3.10)

#include directories
include_directories("${SUCCINCT_INCLUDE_DIR}" 
                    "${LZ4_INCLUDE_DIR}" 
                    "${DSLIB_INCLUDE_DIR}" 
                    "${FST_INCLUDE_DIR}"
                    "./include")

#create executable
add_executable(col_compress col_compress.cpp compress_cols.cpp)
add_dependencies(col_compress succinct-cpp)

#Link libraries
target_link_libraries(col_compress 
                    "${SUCCINCT_LIBRARY}" 
                    "${LZ4_LIBRARY}"
                    "${FST_LIBRARY}" 
                    pthread)

