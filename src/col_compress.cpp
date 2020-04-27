/*
 * Daniel Hodeta
 * File Splitter
 */

#include <iostream>
#include <stdio.h>
#include <fstream>
#include "succinct_file.h"

int splitToCol(char *file_path, int col_num, std::string out_name, bool limit_flag = 0) {

    FILE *fptr = fopen(file_path, "r");
    
    std::ofstream file_out;
    file_out.open(out_name, std::ofstream::app);

    int line_count = 0;
    char *buffer = nullptr;
    size_t n = 0;

    while (getline(&buffer, &n, fptr) != -1) {                          //Populating data table

        if (limit_flag && line_count>6) break;                          //If limit_flag is set to 1, only six lines will be written to file

        std::string line_read {buffer};                                 //Converting from C string to C++ string for more freedom
        line_read = line_read.substr(0, line_read.find('\n'))+" \n";    //Space added before trailing '\n'
        for (int i =0 ; line_read!="\n"; i++) {                         

            std::string cell_data = line_read.substr(0,
                                            line_read.find(' '));       //This is the string we want to store in the table      
            if(i==col_num) {                                            //Write to file
                file_out << cell_data << '\n';
                line_count++;              
            }

            line_read = line_read.substr(line_read.find(' ')+1, 
                                        line_read.size());
        }                                                                      
        buffer = nullptr;                                               //Reinitializing buffer and n for getline
        n = 0;
    }

    file_out.close();

    return 1;

} 

int compressCol(std::string file_name) {
    auto *fd = new SuccinctFile(file_name);

    fd->Serialize(file_name + ".succinct");

    return 1;
}

int main(int argc, char **argv) {

    if (argc<=2) {
        std::cerr << "no file or column number\n";                       //Minimal error checking but alright for our purposes
        exit(1);
    }

    char *file_path {argv[1]};
    int col_num = atoi(argv[2]);                                          //Indicate which column you would like to save
                                                                          //Implemented because splitting all at once takes too much time

    if (col_num<1) {
        std::cerr << "column number must be at least 1\n";
        exit(1);
    }
    
    std::string f_path {file_path};                                      //Converted to C++ String to allow concatenation
    std::string out_name;

    size_t index {f_path.find_last_of("/", f_path.length())};
    if (index == -1){
        out_name = "../data/"+f_path+"_col_"+std::to_string(col_num)+".txt";
    } else {
        out_name = "../data/"+f_path.substr(index+1, f_path.length())+"_col_"+std::to_string(col_num)+".txt";
    }

    std::cout<<out_name<<'\n';

    if(splitToCol(file_path, col_num-1, out_name)) {
        std::cout<<"Splitting Successful!\n";       //Split file
    } else {
        std::cerr<<"Splitting failure";
        exit(1);
    }

    if(compressCol(out_name)) {
        std::cout<<"Compression Successful!\n";
    } else {
        std::cerr<<"Compression failure";
        exit(1);
    }

    return 0;
}