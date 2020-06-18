#!/bin/bash

#This script cleans the directory of all build related files and also deletes all external
#dependencies

./cleanup.sh

cd external
rm -f -r *

echo "cleaned external dependencies"