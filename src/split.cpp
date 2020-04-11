/*
 * Daniel Hodeta
 * Succinct Task 1
 */

#include <iostream>
#include <stdio.h>
#include <vector>
#include <fstream>

//#define LIMIT_LINES

int main(int argc, char **argv) {

    FILE *fptr;
    int col_num = 0;                                                    //Indicate which column you would like to save
                                                                        //Implemented because splitting all at once takes too much time
    if (argc>2) {
        fptr = fopen(argv[1], "r");
        col_num = atoi(argv[2]);
    } else {
        std::cerr << "no file or column number\n";                      //Minimal error checking but alright for our purposes
        exit(1);
    }

    char *buffer = nullptr;
    size_t n = 0;
    
    std::ofstream file_out;
    std::string file_name = "../data/coll_"+std::to_string(col_num+1)+".txt";
    file_out.open(file_name, std::ofstream::app);

    #ifdef LIMIT_LINES
    int count = 0;                                                      //Temporary limit to number of lines read
    while (getline(&buffer, &n, fptr) != -1 && count++<6) {             //Populating data table
    #else
    while (getline(&buffer, &n, fptr) != -1) {                          //Populating data table
    #endif

        std::string line_read {buffer};                                 //Converting from C string to C++ string for more freedom
        line_read = line_read.substr(0, line_read.find('\n'))+" \n";    //Space added before trailing '\n'
        for (int i =0 ; line_read!="\n"; i++) {                         

            std::string cell_data = line_read.substr(0,
                                            line_read.find(' '));       //This is the string we want to store in the table      
            if(i==col_num) {                                            //Write to file
                file_out << cell_data << '\n';              
            }

            line_read = line_read.substr(line_read.find(' ')+1, 
                                        line_read.size());
        }                                                                      
        buffer = nullptr;                                               //Reinitializing buffer and n for getline
        n = 0;
    }
    file_out.close();
    
    fclose(fptr);

    return 0;
}