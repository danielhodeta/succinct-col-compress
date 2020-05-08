#!/bin/bash

#This script compresses the specified column (or all if none given, all columns) of the file given.

#Usage:
#   ./col_compress.sh FILE_NAME [COLUMN_NUMBER]


if [ $# -eq 0 ];                                        #Check file is given
then
    echo "no file given"

elif [ $# -eq 1 ]; 
then 
    col_num=$(head -n 1 $1 | wc -w)                     #Count columns
    for (( i=1; i<=$col_num; i++ ))
    do 
        ./bin/col_compress $@ $i
    done 

else
    if [ $2 -lt 1 ];                                    #Check col_num > 0
    then 
        echo "column number must be at least 1"
    else
        ./bin/col_compress $@
    fi
    
fi
