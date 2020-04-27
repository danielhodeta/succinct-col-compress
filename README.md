# succinct-col-compress
 
This program splits a given file into columns and compresses each column.

**Usage**

    `./build.sh`                            : to build project
    `./col_compress.sh FILE_PATH [COL_NUM]` : to start compressing `FILE`

    `./cleanup.sh`                          : to remove all CMake, build and output files
    `./ext_cleanup.sh`                      : to remove all external dependencies in addition to the files above
 

If `COL_NUM` is not specified, `col_compress` will compress all columns in file

The `col_compress` executable is built in `./bin/`

The output of `col_compress` is stored in `./out/`