#ifndef COMPRESSCOL
#define COMPRESSCOL

#include <stdio.h>

class CompressCols {
    
  private:
    std::string ifile_path;
    std::string ofile_path;
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
    template<typename data_type> int DeltaEAEncode();
    template<typename data_type> void DeltaEADecode();

  public:
    
    CompressCols(std::string file_path, int col_num = 1, bool limit_flag = false);
    int Compress(std::string scheme = "succinct");
    int Decompress();

};

#endif