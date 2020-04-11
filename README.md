# Succinct-Task1
 
split:

    - Usage: 

        make split
        ../build/split file col_num

    - split will save the column in 'file' specified by 'col_num' in "../data/col_[col_num].txt"

split.sh:

    - Usage: 

        ../src/split.sh file col_num

    - Runs split on sample.txt 12 times to split into 12 columns

    NOTE: split.sh assumes that succinct has been built in a folder called "succinct-cpp" in this
    project's parent directory

compress.sh:

    - Usage: 
    
        ../src/compress.sh file col_num

    - Runs the succinct compression executable on each of the 12 columns generated from sample.txt

NOTE:   Before Running any of the above, create a directory 'data' and save sample.txt there. 
        Also, create an empty directory 'build'. Both directories should be on the same level
        as 'src'.
