#!/bin/bash

#This script compresses the specified column (or all if none given, all columns) of the file given.

#Usage:
#   ./col_compress.sh FILE_NAME [COLUMN_NUMBER]

#./build.sh

echo "========================================="


if [ $# -eq 0 ];
then
    echo "no file given"

elif [ $# -eq 1 ]; 
then 
    col_num=$(head -n 1 $1 | wc -w)
    for (( i=1; i<=$col_num; i++ ))
    do 
        echo "==Compressing column $i=="
        ./bin/col_compress $@ $i
    done 

else
    if [ $2 -lt 1 ];
    then 
        echo "column number must be at least 1"
    else
        echo "==Compressing column $2=="
        ./bin/col_compress $@
    fi
    
fi

echo "========================================="
