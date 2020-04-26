cmake_minimum_required(VERSION 3.10)

#include directories
include_directories("${SUCCINCT_INCLUDE_DIR}")

#create executable
add_executable(splitter splitter.cpp)


#Link libraries
target_link_libraries(splitter "${SUCCINCT_LIBRARY}")