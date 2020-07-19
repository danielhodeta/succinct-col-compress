#ifndef COMPRESSCOL
#define COMPRESSCOL

#include <stdio.h>
#include "delta_encoded_array.h"

struct Metadata {
    std::vector<bitmap::EliasGammaDeltaEncodedArray<u_int32_t>*> offsets_vector;
    std::vector<std::vector<int>> type;
    std::vector<std::vector<u_int32_t>> index_in_file;
};

class CompressCols {
    
  private:
    static std::string ifile_path_;
    std::string split_file_path_;
    std::string split_file_name_;
    std::string scheme_;

    int col_num_;
    static u_int64_t line_num_;
    static int total_col_num_;

    std::vector<u_int32_t> metadata_size_;
    std::vector<u_int32_t> unc_size_;
    std::vector<u_int32_t> run_size_;

    Metadata* meta_data_;

    static bool split_;
    bool compressed_;
    static bool limit_flag_;

    int SuccinctCompress();
    int LZ4Compress();
    void LZ4Decompress();
    template<typename T> int DeltaEAEncode();
    template<typename T> void DeltaEADecode();
    template<typename T> T DeltaEAIndexAt(int file_type, u_int32_t index,
                                            bool get_flag = false,
                                            bitmap::EliasGammaDeltaEncodedArray<T>** get_dec_array = nullptr,
                                            T* get_run_data = nullptr,
                                            T* get_unc_data = nullptr,
                                            int64_t get_index = -1,
                                            int type = -1);
    template<typename T> int DeaRleEncodeArray(T* data_array, std::string file_path, std::string file_name);
    template<typename T> int QueryBinarySearch (T key, u_int32_t& l_index, u_int32_t& u_index);
    Metadata* ReadMetadata(int file_type, std::string& delta_file_path);
    int64_t TypeBinarySearch (u_int32_t metadata_size, bitmap::EliasGammaDeltaEncodedArray<u_int32_t>& offset, 
                                            u_int64_t value); 

  public:
    static int Split(std::string file_path = ifile_path_, int total_col_num = total_col_num_);
    CompressCols(std::string file_path, int total_col_num=12, int col_num = 1, bool limit_flag = false);
    int Compress(std::string scheme = "succinct");
    int Decompress();
    int SingleKeyLookup (u_int64_t key, std::vector<u_int32_t>* indices);
    friend void Benchmarking (CompressCols* c_file, std::vector<u_int64_t>& queries);

};

#endif