# Succinct-Task1
 
Task1:

    - Usage: 

        make Task1
        ../build/Task1 file col_num

    - Task1 will save the column in 'file' specified by 'col_num' in "../data/col_[col_num].txt"

split.sh:

    - Usage: 

        ../src/split.sh file col_num

    - Runs Task1 on sample.txt 12 times to split into 12 columns

compress.sh:

    - Usage: 
    
        ../src/compress.sh file col_num

    - Runs the succinct compression executable on each of the 12 columns generated from sample.txt

NOTE:   Before Running any of the above, create a directory 'data' and save sample.txt there. 
        Also, create an empty directory 'build'. Both directories should be on the same level
        as 'src'.
