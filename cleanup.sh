#!/bin/bash

#This script cleans all Cmake related and build related files

rm -f -r CMakeFiles
rm -f cmake_install.cmake
rm -f Makefile

rm -f -r src/CMakeFiles
rm -f src/cmake_install.cmake
rm -f src/Makefile

rm -f CMakeCache.txt

rm -f -r build/

rm -f -r bin/

rm -f -r out/