#!/bin/bash

#This script cleans all Cmake related and build related files

rm -f -r CMakeFiles
rm -f cmake_install.cmake
rm -f Makefile
rm -f CMakeCache.txt
echo "cleaned cmake files in parent directory"

rm -f -r src/CMakeFiles
rm -f src/cmake_install.cmake
rm -f src/Makefile
echo "cleaned cmake files in src directory"

rm -f -r build
echo "cleaned build"

rm -f -r bin
echo "cleaned bin"

rm -f -r out
echo "cleaned out"