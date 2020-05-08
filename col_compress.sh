#!/bin/bash

#This script compresses the specified column (or all if none given, all columns) of the file given.

#Usage:
#   ./col_compress.sh FILE_NAME [-c COLUMN_NUMBER] [COMPRESSION SCHEME]*

schemes=("succinct" "lz4" "dea")

if [ $# -eq 0 ];                                           #Check file is given
then
    echo "no file given"

elif [ $# -eq 1 ];                                         #Only file name given
then 
    col_num=$(head -n 1 $1 | wc -w)                     
    for (( i=1; i<=$col_num; i++ ))
    do 
        ./bin/col_compress $@ $i ${schemes[@]}
        echo
    done  

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
    else                                                    #File name and column number given
        col_num=$(head -n 1 $1 | wc -w)                     
        for (( i=1; i<=$col_num; i++ ))
        do 
            ./bin/col_compress $1 $i ${@:2}
            echo
        done 
    fi
    
fi
