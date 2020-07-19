#!/bin/bash

#This script compresses the specified column (or all if none given, all columns) of the file given.

#Usage:
#   ./col_compress.sh FILE_NAME [-c COLUMN_NUMBER] [COMPRESSION SCHEME]*

rm -f -r ./out/*                                           #Clean output folder
rm -f -r ./bench/bench_array_indices

schemes=("succinct" "lz4" "dea")
TOTAL_COLS=$(head -n 1 $1 | wc -w) 

if [ $# -eq 0 ];                                           #Check file is given
then
    echo "no file given"

elif [ $# -eq 1 ];                                         #Only file name given
then 
                         
    ./bin/col_compress $@ $TOTAL_COLS ${schemes[@]}
    echo 

else
    if [ $# -gt 3 ] && [ $2 = '-c' ];                      #File name and column number given              
    then
        if [ $3 -le 0 ];                                   #Check col_num > 0
        then 
            echo "column number must be at least 1"
        else
            ./bin/col_compress $1 $3 ${@:4}
            echo
        fi

    elif [ $# -lt 3 ] && [ $2 = '-c' ];                     #Column number flag without actual number ->error
    then
        echo "column number not specified"

    elif [ $# -eq 3 ] && [ $2 = '-c' ];                     #Column number flag without actual number ->error
    then
        if [ $3 -le 0 ];                                    #Check col_num > 0
        then 
            echo "column number must be at least 1"
        else
            ./bin/col_compress $1 $3 ${schemes[@]}
            echo
        fi
    else                                                    #File name and compression schemes given                    
        ./bin/col_compress $1 $TOTAL_COLS ${@:2}
        echo
    fi
    
fi
