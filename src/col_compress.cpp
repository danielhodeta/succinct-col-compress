/*
 * Daniel Hodeta
 * File Splitter
 */

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include "succinct_file.h"
#include "LZ4Compress.h"

void display_error(std::string msg) {
    std::cout<<"col_compress: "<<msg<<'\n';
    exit(1);
}

int SplitToCol(char *file_path, int col_num, std::string out_name, bool limit_flag = 0) {
    
    FILE *fptr = fopen(file_path, "r");
    
    if (!fptr) {
        std::cerr<<"splitToCol: invalid file!\n";
        return 0;
    }
    
    std::ofstream file_out;
    file_out.open(out_name, std::ofstream::trunc);


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

int SuccinctCompress(std::string file_name) {
    auto *fd = new SuccinctFile(file_name);

    fd->Serialize(file_name + ".succinct");

    return 1;
}


int main(int argc, char **argv) {

    if (argc<=2) display_error("no file or column number");                  //Minimal error checking but alright for our purposes

    char *file_path {argv[1]};
    
    int col_num = atoi(argv[2]);                                            //Indicate which column you would like to save

    if (col_num<1) display_error("column number must be at least 1");
    
    std::string f_path {file_path};                                         //Converted to C++ String to allow concatenation
    std::string out_name;

    size_t index {f_path.find_last_of("/", f_path.length())};
    if (index == -1)
        out_name = "./out/"+f_path+"_col_"+std::to_string(col_num)+".txt";
    else
        out_name = "./out/"+f_path.substr(index+1, f_path.length())+"_col_"+std::to_string(col_num)+".txt";

    
    if (SplitToCol(file_path, col_num-1, out_name))         //Split file
        std::cout<<"splitting successful!\n";       
    else
        display_error("splitting failure!");

    
    if (SuccinctCompress(out_name))                              //Compress file
        std::cout<<"compression successful!\n"; 
    else
        display_error("compression failure!");


    return 0;
}