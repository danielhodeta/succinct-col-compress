/*
 * Daniel Hodeta
 * File Splitter
 */

#include <iostream>
#include <stdio.h>
#include <fstream>
#include "compress_cols.h"

void display_error(std::string msg) {
    std::cout<<"col_compress: "<<msg<<'\n';
    exit(1);
}


int main(int argc, char **argv) {

    if (argc<=2) display_error("no file or column number");                  //Minimal error checking but alright for our purposes

    char *file_path {argv[1]};
    std::string f_path {file_path};                                         //Converted to C++ String to allow concatenation

    int total_col_num = atoi(argv[2]);                                      //Indicate total col number
    if (total_col_num<1) display_error("total column number must be at least 1");
    CompressCols::Split(f_path, 12);
    for (int i=1; i<=total_col_num; i++) {
        
        auto *cfile = new CompressCols(f_path, total_col_num, i);

        for (int j=3; j<argc; j++) {
            std::string scheme = argv[j];
            
                if(!cfile->Compress(scheme))
                    std::cout<<scheme + " compression unsuccessful for column "+std::to_string(i)+".\n";
                if(cfile->Decompress())
                    std::cout<<scheme + " decompression successful for column "+std::to_string(i)+".\n";
                else
                    std::cout<<scheme + " decompression unsuccessful for column "+std::to_string(i)+".\n";

        }

        delete cfile;
    }

    
    

    

    return 0;
}