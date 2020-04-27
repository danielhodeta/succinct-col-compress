#!/bin/bash

#This script compresses the specified column (or all if none given, all columns) of the file given.

#Usage:
#   ./col_compress.sh FILE_NAME [COLUMN_NUMBER]

./build.sh

if [ $# -eq 1 ]; 
then 
    col_num=$(head -n 1 $1 | wc -w)
    for (( i=1; i<=$col_num; i++ ))
    do 
        echo compressing column $i
        ./bin/col_compress $@ $i
    done 

else
    echo compressing column $2
    ./bin/col_compress $@
fi 