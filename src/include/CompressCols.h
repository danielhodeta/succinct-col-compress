#ifndef COMPRESSCOL
#define COMPRESSCOL

#include <stdio.h>

class CompressCols {
    
  private:
    std::string ifile_path;
    std::string split_file_path;
    std::string split_file_name;
    std::string scheme;

    int col_num;
    u_int64_t line_num;

    bool split;
    bool compressed;
    bool limit_flag;

    int Split();
    int SuccinctCompress();
    int LZ4Compress();
    void LZ4Decompress();
    template<typename T> int DeltaEAEncode();
    template<typename T> void DeltaEADecode();
    template<typename T> T DeltaEAIndexAt(u_int64_t index);

  public:
    
    CompressCols(std::string file_path, int col_num = 1, bool limit_flag = false);
    int Compress(std::string scheme = "succinct");
    int Decompress();

};

#endif