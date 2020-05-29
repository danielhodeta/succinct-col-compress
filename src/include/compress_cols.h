#ifndef COMPRESSCOL
#define COMPRESSCOL

#include <stdio.h>

class CompressCols {
    
  private:
    static std::string ifile_path_;
    std::string split_file_path_;
    std::string split_file_name_;
    std::string scheme_;

    int col_num_;
    static u_int64_t line_num_;
    static int total_col_num_;

    static bool split_;
    bool compressed_;
    static bool limit_flag_;

    int SuccinctCompress();
    int LZ4Compress();
    void LZ4Decompress();
    template<typename T> int DeltaEAEncode();
    template<typename T> void DeltaEADecode();
    template<typename T> T DeltaEAIndexAt(u_int64_t index);

  public:
    static int Split(std::string file_path = ifile_path_, int total_col_num = total_col_num_);
    CompressCols(std::string file_path, int total_col_num=12, int col_num = 1, bool limit_flag = false);
    int Compress(std::string scheme = "succinct");
    int Decompress();

};

#endif