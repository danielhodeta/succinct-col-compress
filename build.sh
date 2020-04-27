#!/bin/bash

#This script builds the project

mkdir -p build
mkdir -p out
cd build
cmake ../
make