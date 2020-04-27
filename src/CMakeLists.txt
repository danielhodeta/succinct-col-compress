cmake_minimum_required(VERSION 3.10)

#include directories
include_directories("${SUCCINCT_INCLUDE_DIR}")

#create executable
add_executable(col_compress col_compress.cpp)
add_dependencies(col_compress succinct-cpp)

#Link libraries
target_link_libraries(col_compress "${SUCCINCT_LIBRARY}" pthread)