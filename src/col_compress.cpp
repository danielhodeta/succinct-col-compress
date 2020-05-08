/*
 * Daniel Hodeta
 * File Splitter
 */

#include <iostream>
#include <stdio.h>
#include <fstream>
#include "CompressCols.h"

void display_error(std::string msg) {
    std::cout<<"col_compress: "<<msg<<'\n';
    exit(1);
}


int main(int argc, char **argv) {

    if (argc<=2) display_error("no file or column number");                  //Minimal error checking but alright for our purposes

    char *file_path {argv[1]};
    std::string f_path {file_path};                                         //Converted to C++ String to allow concatenation
    

    int col_num = atoi(argv[2]);                                            //Indicate which column you would like to save
    if (col_num<1) display_error("column number must be at least 1");
    
    std::string scheme = "lz4";
    auto *cfile = new CompressCols(f_path, col_num);

    if(cfile->Compress(scheme))
        std::cout<<"Compressed column "+std::to_string(col_num)+" using "+scheme+".\n";
    else
        std::cout<<"Compression unsuccessful.\n";

    if(cfile->Decompress())
        std::cout<<"Decompression successful.\n";
    else
        std::cout<<"Decompression unsuccessful.\n";

    return 0;
}