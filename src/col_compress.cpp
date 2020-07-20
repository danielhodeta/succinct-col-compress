/*
 * Daniel Hodeta
 * File Splitter
 */

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <chrono>
#include <sys/stat.h> 
#include "compress_cols.h"
#include "delta_encoded_array.h"

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

    std::vector<CompressCols*> cfiles;
    cfiles.reserve(12);
    for (int i=4; i<=total_col_num; i++) {                          //Indexing starts at one
        
        auto *cfile = new CompressCols(f_path, total_col_num, i);

        for (int j=3; j<argc; j++) {
            std::string scheme = argv[j];
            
                if(!cfile->Compress(scheme))
                    std::cout<<scheme + " compression unsuccessful for column "+std::to_string(i)+".\n";
                // if(cfile->Decompress())
                //     std::cout<<scheme + " decompression successful for column "+std::to_string(i)+".\n";
                // else
                //     std::cout<<scheme + " decompression unsuccessful for column "+std::to_string(i)+".\n";

        }
        cfiles[i-1] = cfile;
    }
    

    int query_size = 1000;
    FILE *col_4_file = fopen ("./bench/queries/test_col_4", "r");
    u_int64_t* col_4_q = new u_int64_t[query_size];
    size_t col_4_read_size = fread(col_4_q, sizeof(u_int64_t), query_size, col_4_file);
    assert (col_4_read_size == query_size);
    std::vector<u_int64_t> col_4_queries;    
    for(int i=0; i<query_size; i++) col_4_queries.push_back(col_4_q[i]);

    FILE *col_5_file = fopen ("./bench/queries/test_col_5", "r");
    u_int64_t* col_5_q = new u_int64_t[query_size];
    size_t col_5_read_size = fread(col_5_q, sizeof(u_int64_t), query_size, col_5_file);
    assert (col_5_read_size == query_size);
    std::vector<u_int64_t> col_5_queries;    
    for(int i=0; i<query_size; i++) col_5_queries.push_back(col_5_q[i]);

    FILE *col_6_file = fopen ("./bench/queries/test_col_6", "r");
    u_int64_t* col_6_q = new u_int64_t[query_size];
    size_t col_6_read_size = fread(col_6_q, sizeof(u_int64_t), query_size, col_6_file);
    assert (col_6_read_size == query_size);
    std::vector<u_int64_t> col_6_queries;    
    for(int i=0; i<query_size; i++) col_6_queries.push_back(col_6_q[i]);

    FILE *col_7_file = fopen ("./bench/queries/test_col_7", "r");
    u_int64_t* col_7_q = new u_int64_t[query_size];
    size_t col_7_read_size = fread(col_7_q, sizeof(u_int64_t), query_size, col_7_file);
    assert (col_7_read_size == query_size);
    std::vector<u_int64_t> col_7_queries;    
    for(int i=0; i<query_size; i++) col_7_queries.push_back(col_7_q[i]);


    //Benchmarking(cfiles[3], col_4_queries);
    //Benchmarking(cfiles[4], col_5_queries);
    //Benchmarking(cfiles[5], col_6_queries);
     Benchmarking(cfiles[6], col_7_queries);

    return 0;
}

void Benchmarking (CompressCols* c_file, std::vector<u_int64_t>& queries) {
    std::string file_name = "./bench/bench_";
    int col_num = c_file->col_num_;
    std::ofstream results_stream (file_name+std::to_string(col_num), std::ofstream::trunc);
    std::ofstream indices_stream (file_name+"dea_indices", std::ofstream::app);

    //Warmup
    std::cerr<<"\nWarming up\n";
    for (int i=0; i<std::min(queries.size(), 100UL); i++) {
        std::vector<u_int32_t> indices;
        c_file->SingleKeyLookup(queries[i], &indices);
    }
    std::cerr<<"Warming up done\n";

    std::cerr<<"Measuring\n";
    for (int i=0; i<queries.size(); i++) {
        std::vector<u_int32_t> indices;
        std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
        c_file->SingleKeyLookup(queries[i], &indices);
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        long double time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
        results_stream<<time_span<<"\n";
        
        for (int j=0; j<indices.size(); j++) indices_stream<<indices[j]<<"\n";
    }
    std::cerr<<"Measuring done\n";
    results_stream.close();
    indices_stream.close();
}