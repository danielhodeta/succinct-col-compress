#include <stdio.h>
#include <cstring>
#include <sstream>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "succinct_file.h"
#include "lz4frame.h"
#include "compress_cols.h"
#include "delta_encoded_array.h"
#include <sys/stat.h>
#include <sys/time.h>

//Static Variable Definition

bool CompressCols::split_ = false;
u_int64_t CompressCols::line_num_ = 0; 
int CompressCols::total_col_num_ = 0;
bool CompressCols::limit_flag_ = 0;
std::string CompressCols::ifile_path_ = "";

//Time Stamp
typedef unsigned long long int timestamp_t;

static timestamp_t get_timestamp() {
  struct timeval now{};
  gettimeofday(&now, nullptr);

  return (now.tv_usec + (time_t) now.tv_sec * 1000000);
}

//Class Contructor
CompressCols::CompressCols(std::string file_path, int total_col_num, int col_num, bool limit_flag) {
    
    ifile_path_ = file_path;
    this->col_num_ = col_num;
    total_col_num_ = total_col_num;

    size_t index {file_path.find_last_of("/", file_path.length())};
    if (index == -1) {
        this->split_file_name_ = file_path+"_col_"+std::to_string(col_num)+".txt";
        this->split_file_path_ = "./out/"+this->split_file_name_;
    } else {
        this->split_file_name_ = file_path.substr(index+1, file_path.length())+"_col_"+std::to_string(col_num)+".txt";
        this->split_file_path_ = "./out/"+this->split_file_name_; 
    }
    
    this->limit_flag_ = limit_flag;
    this->compressed_ = false;
    
    
}

//Compression function
int CompressCols::Compress(std::string scheme) {
    if (!split_) this->Split();
    
    this->scheme_ = scheme;

    timestamp_t start,total_time;
    int return_val = 0;

    switch (this->scheme_.c_str()[0]) {
        case 's':
            if (this->scheme_!="succinct") break;

            start = get_timestamp();
            return_val = this->SuccinctCompress();
            total_time = get_timestamp() - start;
            
            if (return_val) {
                std::cout<<"compressed column "+std::to_string(col_num_)+" using "+scheme_+",";
                std::cout<<" time taken: "<<total_time/1000000.0<<"\n";
            }

            this->compressed_ = true;
            break;
        case 'l':
            if (this->scheme_!="lz4") break;

            start = get_timestamp();
            return_val = this->LZ4Compress();
            total_time = get_timestamp() - start;
            
            if (return_val) {
                std::cout<<"compressed column "+std::to_string(col_num_)+" using "+scheme_+",";
                std::cout<<" time taken: "<<total_time/1000000.0<<"\n";
            }

            this->compressed_ = true;
            break;
        case 'd':
            if (this->scheme_!="dea") break;

            start = get_timestamp(); 
            return_val = this->DeltaEAEncode<u_int64_t>();
            total_time = get_timestamp() - start;

            if (return_val) {
                std::cout<<"compressed column "+std::to_string(col_num_)+" using "+scheme_+",";
                std::cout<<" time taken: "<<total_time/1000000.0<<"\n";
            }

            this->compressed_ = true;
            break;
        default:
            break;
    }
    return return_val;
}

//Decompression function (using original compression scheme)
int CompressCols::Decompress() {

    int return_val (0);

    if (!this->compressed_) {
        std::cerr<<"File hasn't been compressed yet!\n";
        return return_val;
    }

    switch (this->scheme_.c_str()[0]) {
        case 's':
            //return_val = this->SuccinctDecompress();
            break;
        case 'l':
            if (this->scheme_!="lz4") break;
            this->LZ4Decompress();
            return_val = 1;
            break;
        case 'd':
            if (this->scheme_!="dea") break;
            this->DeltaEADecode<u_int64_t>();
            return_val = 1;
            break;
        default:
            //return_val = this->SuccinctCompress();
            break;
    }

    return return_val;

}

//File splitting function
int CompressCols::Split(std::string file_path, int total_col_num) {

    timestamp_t start = get_timestamp();

    size_t index {file_path.find_last_of("/", file_path.length())};
    std::string split_file_path_prefix;
    if (index == -1) {
        split_file_path_prefix = "./out/"+file_path+"_col_";
    } else {
        split_file_path_prefix = "./out/"+file_path.substr(index+1, file_path.length())+"_col_"; 
    }

    std::vector<std::ofstream*> split_file_output_streams;
    split_file_output_streams.reserve(total_col_num);
    
    for (int i=0; i<total_col_num; i++) {
        std::string file_path = split_file_path_prefix+std::to_string(i+1)+".txt";
        std::ofstream *file_stream = new std::ofstream(file_path, std::ofstream::trunc);
        split_file_output_streams.push_back(file_stream);
    }
    

    FILE *fptr = fopen(file_path.c_str(), "r");
    
    if (!fptr) {
        std::cerr<<"SplitToCol: invalid file!\n";
        exit(1);
    }
    
    // std::ofstream file_out;
    // file_out.open(sfp, std::ofstream::trunc);


    int line_count = 0;
    char *buffer = nullptr;
    size_t n = 0;

    while (getline(&buffer, &n, fptr) != -1) {                          //Populating data table

        if (limit_flag_ && line_count>6) break;                    //If limit_flag is set to 1, only six lines will be written to file

        std::string line_read {buffer};                                 //Converting from C string to C++ string for more freedom
        line_read = line_read.substr(0, line_read.find('\n'))+" \n";    //Space added before trailing '\n'
        for (int i =0 ; i<total_col_num && line_read!="\n"; i++) {                         

            std::string cell_data = line_read.substr(0,
                                            line_read.find(' '));       //This is the string we want to store in the table      
            
            while (cell_data.compare("")==0) {                            //Skip empty strings
                line_read = line_read.substr(line_read.find(' ')+1, 
                                        line_read.size());
                cell_data = line_read.substr(0,
                                            line_read.find(' '));
            }
            if(i<=total_col_num) {                                            //Write to file
                *split_file_output_streams[i] << cell_data << '\n';              
            }

            line_read = line_read.substr(line_read.find(' ')+1, 
                                        line_read.size());
        }   
        line_count++;
        free(buffer);                                                                   
        buffer = nullptr;                                               //Reinitializing buffer and n for getline
        n = 0;

    }
    free(buffer);
    line_num_ = line_count;
    
    for (int i=0; i<total_col_num; i++) {
        (*split_file_output_streams[i]).close();
        delete split_file_output_streams[i];
    }
    fclose(fptr);
    split_ = true;

    timestamp_t total_time = get_timestamp() - start;
    std::cout<<"split columns successfully, time taken: "<<total_time/1000000.0<<"\n";
    return 1;

}


/* ========================
 *   Compression Schemes
 * ========================
 */


/* Succinct */

int CompressCols::SuccinctCompress() {
    std::string sfp = this->split_file_path_;
    auto *fd = new SuccinctFile(sfp);

    fd->Serialize(sfp + ".succinct");

    return 1;
}

/*
* LZ4
* 
* Code Source: lz4/examples/frameCompress.c
*/ 

#define IN_CHUNK_SIZE  (16*1024)

static const LZ4F_preferences_t kPrefs = {
    { LZ4F_max256KB, LZ4F_blockLinked, LZ4F_noContentChecksum, LZ4F_frame,
      0 /* unknown content size */, 0 /* no dictID */ , LZ4F_noBlockChecksum },
    0,   /* compression level; 0 == default */
    0,   /* autoflush */
    0,   /* favor decompression speed */
    { 0, 0, 0 },  /* reserved, must be set to 0 */
};


/* safe_fwrite() :
 * performs fwrite(), ensure operation success, or immediately exit() */
static void safe_fwrite(void* buf, size_t eltSize, size_t nbElt, FILE* f)
{
    size_t const writtenSize = fwrite(buf, eltSize, nbElt, f);
    size_t const expectedSize = eltSize * nbElt;
    if (nbElt > 0) assert(expectedSize / nbElt == eltSize);   /* check overflow */
    if (writtenSize < expectedSize) {
        if (ferror(f))  /* note : ferror() must follow fwrite */
            ;//fprintf(stderr, "Write failed \n");
        else
            ;//fprintf(stderr, "Short write \n");
        exit(1);
    }
}

typedef struct {
    int error;
    unsigned long long size_in;
    unsigned long long size_out;
} compressResult_t;

static compressResult_t
compress_file_internal(FILE* f_in, FILE* f_out,
                       LZ4F_compressionContext_t ctx,
                       void* inBuff,  size_t inChunkSize,
                       void* outBuff, size_t outCapacity)
{
    compressResult_t result = { 1, 0, 0 };  /* result for an error */
    unsigned long long count_in = 0, count_out;

    assert(f_in != NULL); assert(f_out != NULL);
    assert(ctx != NULL);
    assert(outCapacity >= LZ4F_HEADER_SIZE_MAX);
    assert(outCapacity >= LZ4F_compressBound(inChunkSize, &kPrefs));

    /* write frame header */
    {   size_t const headerSize = LZ4F_compressBegin(ctx, outBuff, outCapacity, &kPrefs);
        
        if (LZ4F_isError(headerSize)) {
            return result;
        }
        count_out = headerSize;
        safe_fwrite(outBuff, 1, headerSize, f_out);
    }
    /* stream file */
    for (;;) {
        size_t const readSize = fread(inBuff, 1, IN_CHUNK_SIZE, f_in);
        if (readSize == 0) break; /* nothing left to read from input file */
        count_in += readSize;
        size_t const compressedSize = LZ4F_compressUpdate(ctx,
                                                outBuff, outCapacity,
                                                inBuff, readSize,
                                                NULL);
        if (LZ4F_isError(compressedSize)) {
            return result;
        }
        safe_fwrite(outBuff, 1, compressedSize, f_out);
        count_out += compressedSize;
    }
    /* flush whatever remains within internal buffers */
    {   size_t const compressedSize = LZ4F_compressEnd(ctx,
                                                outBuff, outCapacity,
                                                NULL);
        if (LZ4F_isError(compressedSize)) {
            return result;
        }
        safe_fwrite(outBuff, 1, compressedSize, f_out);
        count_out += compressedSize;
    }

    result.size_in = count_in;
    result.size_out = count_out;
    result.error = 0;
    return result;
}

int CompressCols::LZ4Compress() {
    std::string sfp = this->split_file_path_;

    FILE * const f_in = fopen(sfp.c_str(), "rb");
    FILE * const f_out = fopen((sfp+".lz4").c_str(), "wb");

    assert(f_in != NULL);
    assert(f_out != NULL);

    /* ressource allocation */
    LZ4F_compressionContext_t ctx;
    size_t const ctxCreation = LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
    void* const src = malloc(IN_CHUNK_SIZE);
    size_t const outbufCapacity = LZ4F_compressBound(IN_CHUNK_SIZE, &kPrefs);   /* large enough for any input <= IN_CHUNK_SIZE */
    void* const outbuff = malloc(outbufCapacity);

    compressResult_t result = { 1, 0, 0 };  /* == error (default) */
    if (!LZ4F_isError(ctxCreation) && src && outbuff) {
        result = compress_file_internal(f_in, f_out,
                                        ctx,
                                        src, IN_CHUNK_SIZE,
                                        outbuff, outbufCapacity);
    } else {
        return 0;
    }

    LZ4F_freeCompressionContext(ctx);   /* supports free on NULL */
    free(src);
    free(outbuff);
    // free(f_in);
    // free(f_out);
    return 1;
}

//LZ4 Decompression function
 void CompressCols::LZ4Decompress() {
//     std::string ofp = this->ofile_path;
    
//     FILE *inpFp = fopen((ofp+".lz4").c_str(), "rb");
//     FILE *outFp = fopen((ofp+".lz4.dec").c_str(), "wb");

//     enum {
//         messageMaxBytes   = 1024,
//         ringBufferBytes   = 1024 * 256 + messageMaxBytes,
//     };

//     LZ4_streamDecode_t* const lz4StreamDecode = LZ4_createStreamDecode();
//     char* const cmpBuf = (char*) malloc(LZ4_COMPRESSBOUND(messageMaxBytes));
//     char* const decBuf = (char*) malloc(ringBufferBytes);
//     int decOffset = 0;

//     for ( ; ; )
//     {
//         uint16_t cmpBytes = 0;

//         if (read_uint16(inpFp, &cmpBytes) != 1) break;
//         if (cmpBytes == 0) break;
//         if (read_bin(inpFp, cmpBuf, cmpBytes) != cmpBytes) break;

//         {
//             char* const decPtr = &decBuf[decOffset];
//             const int decBytes = LZ4_decompress_safe_continue(
//                 lz4StreamDecode, cmpBuf, decPtr, cmpBytes, (int) messageMaxBytes);
//             if (decBytes <= 0) break;
//             write_bin(outFp, decPtr, decBytes);

//             // Add and wraparound the ringbuffer offset
//             decOffset += decBytes;
//             if ((size_t)decOffset >= ringBufferBytes - messageMaxBytes) decOffset = 0;
//         }
//     }

//     fclose(inpFp);
//     fclose(outFp);
//     free(decBuf);
//     free(cmpBuf);
//     LZ4_freeStreamDecode(lz4StreamDecode);
 }

/* Delta Endoded Array */

template<typename T>
static T* MergeSort (T* data_array, u_int64_t size) {

    if (size == 1) {
        return data_array;
    } else if (size == 2) {

        if (data_array[0] > data_array[1]) {
            T temp = data_array[1];
            data_array[1] = data_array[0];
            data_array[0] = temp;
        }
        return data_array;
    } else {
        u_int64_t midpoint = (size - 1)/2; //left = 0-mid (mid+1)  right = mid+1 - size-1 (size-1-mid)
        T* left = MergeSort(data_array, midpoint+1);
        T* right = MergeSort(data_array+(midpoint+1), size-(midpoint+1));

        u_int64_t l=0; 
        u_int64_t r=0;

        T* sorted = new T[size];

        for (u_int64_t i=0; i<size; i++) {
            if (r >= size-midpoint-1) {sorted[i] = left[l++]; continue;}
            if (l >= midpoint+1) {sorted[i] = right[r++]; continue;}

            if (left[l] < right[r]) sorted[i] = left[l++];
            else sorted[i] = right[r++];
            
        }
        if (midpoint+1 > 2) delete[] left;
        if (size-(midpoint+1) > 2) delete[] right;
        // delete[] left;
        // delete[] right;
        assert(r+l == size);
        return sorted;

    }
}

//DEA_Encoding and Serialization
template<typename T>
int CompressCols::DeltaEAEncode() {

    std::string delta_fp = "./out/"+this->split_file_name_+".dea/";
    if(mkdir(delta_fp.c_str(), 0777) == -1) return 0;

    std::ifstream file_in (this->split_file_path_);                                                //Reading split file
    std::string read_num;
    T *data_array = new T[line_num_];                                   
    for (uint64_t i=0; i<line_num_ && (file_in >> read_num); i++) {                           //Converting strings to T and storing in array

        std::istringstream read_stream (read_num);
        read_stream >> data_array[i];

    }
    file_in.close();

    //Sort
    T* sorted_data = MergeSort<T>(data_array, line_num_);
    delete[] data_array;
    data_array = sorted_data;


    std::ofstream metadata (delta_fp+"metadata", std::ofstream::out);
    std::ofstream dea (delta_fp+this->split_file_name_+".dea");
    std::ofstream run (delta_fp+this->split_file_name_+".run");
    std::ofstream uncompressed (delta_fp+this->split_file_name_+".unc");

    //Metadata
    u_int64_t offset = 0;
    //int type = 0;                               //0 => dea, 1= run, 2=uncompressed
    u_int64_t len = 1;
    //u_int64_t index = 0;

    //Counters
    u_int64_t dea_count = 0;
    u_int64_t run_count = 0;
    u_int64_t uncompressed_count = 0;                                     

    int run_threshold =5;

    //States
    bool sorted = 0;
    bool in_run = 0;

    auto ini_type {
        [&](int i) {
            if (i<line_num_-1 && data_array[i] < data_array[i+1]) {
                sorted = 1;
            } else {                                                //sorted so > can be ignored
                in_run = 1;
            }
            offset = i;
        }
    };

    auto dea_end {
        [&] {
            bitmap::EliasGammaDeltaEncodedArray<T> enc_array(data_array+offset, len);               //Encoding array
            enc_array.Serialize(dea);                                                               //Serializing array

            metadata<<offset<<" "<<0<<" "<<dea_count<<"\n";

            dea_count++;
            sorted = 0;
            len = 1;
        }
    };

    auto run_end {
        [&] {

            run<<data_array[offset]<<"\n";
            metadata<<offset<<" "<<1<<" "<<run_count<<"\n";

            len = 1;
            run_count++;
            in_run=0;  
        }
    };

    auto unc_end {
        [&] {
            for (int i=0; i<len; i++) uncompressed<<data_array[offset+i]<<"\n";

            metadata<<offset<<" "<<2<<" "<<uncompressed_count<<"\n";

            uncompressed_count+=len;
            len=1;
            sorted=0;
        }
    };

    for (u_int64_t i=0; i<line_num_; i++) {
        if (i==0) {
            ini_type(i);
        } else {
            assert(in_run!=sorted);

            int comp;
            if (data_array[i-1] < data_array[i]) comp = 1;
            else comp = 0;

            if (in_run) {
                if (!comp) {
                    len++;
                } else {
                    run_end();
                    ini_type(i);
                }
            } else {
                if (comp) {
                    len++;
                } else {
                    if (len<run_threshold) unc_end();
                    else dea_end();

                    ini_type(i);
                }
            }

            if (i==line_num_-1) {
                if (in_run) {
                    run_end();
                } else {
                    if (len<run_threshold) unc_end();
                    else dea_end();
                } 
            }
        }
    }
        
    // metadata<<std::to_string(this->line_num_)<<"\n";                                 //Sentry
    // metadata<<"-1";

    uncompressed.close();
    run.close();
    dea.close();
    metadata.close();

    delete[] data_array;
    return 1;
}

static int64_t BinarySearch (std::vector<u_int64_t> offset, u_int64_t index) {
    u_int64_t l_bound = 0;
    u_int64_t r_bound = offset.size()-1;
    u_int64_t midpoint;

    if (index >= offset[l_bound] && index < offset[l_bound+1]) return l_bound;
    if (index >= offset[r_bound]) return r_bound;

    while (l_bound<=r_bound) {
        midpoint = l_bound + ((r_bound - l_bound)/2);
        if (index<offset[midpoint]) {
            if (index>=offset[midpoint-1]) return midpoint-1;
            else r_bound = midpoint-2;
        } else {
            if(index<offset[midpoint+1]) return midpoint;
            else l_bound = midpoint+1;
        }
    }
    return -1;
} 

//DEA Deserialization
template<typename T>
T CompressCols::DeltaEAIndexAt(u_int64_t index) {

    std::string delta_fp = "./out/"+this->split_file_name_+".dea/";
      

    std::ifstream metadata (delta_fp+"metadata");

    //Metadata
    static std::vector<u_int64_t> offset {};
    static std::vector<int> type {};
    static std::vector<u_int16_t> index_in_file {};

    u_int64_t data;
    if (offset.size()==0) {
        while(metadata.peek()!=EOF) {
            metadata >> data;
            offset.push_back(data);

            metadata >> data;
            type.push_back(data);

            metadata >> data;
            index_in_file.push_back(data);
            
        }
    }
    assert((offset.size()-type.size()+index_in_file.size())==offset.size());               //making sure they're the same size()
    
    if (offset[offset.size()-2] > offset[offset.size()-1]) {                            //Erase last entry if it's meaningless
        offset.erase(offset.end()-1);
        type.erase(type.end()-1);
        index_in_file.erase(index_in_file.end()-1);
    }   

    int64_t position = BinarySearch(offset, index);
    assert(position>=0);

    static std::vector<bitmap::EliasGammaDeltaEncodedArray<T>*> dec_arrays {};
    static std::vector<u_int64_t> runs {};
    static std::vector<u_int64_t> unc_data {};

    if (type[position]==0) {
        
        if (dec_arrays.size()==0){
            std::ifstream dea (delta_fp+this->split_file_name_+".dea");
            while (dea.peek()!=EOF) {
                auto temp = new bitmap::EliasGammaDeltaEncodedArray<T>(NULL, 0);
                (*temp).Deserialize(dea);
                dec_arrays.push_back(temp);
            }
            dea.close();
        }
        return (*dec_arrays[index_in_file[position]])[index-offset[position]];

    } else if (type[position]==1) {
        if (runs.size()==0) {
            std::ifstream run (delta_fp+this->split_file_name_+".run");
            while (run.peek()!=EOF) {
                u_int64_t temp;
                run >> temp;
                runs.push_back(temp);
            }
            run.close();
        }
        return runs[index_in_file[position]];

    } else {
        if (unc_data.size()==0) {
            std::ifstream uncompressed (delta_fp+this->split_file_name_+".unc"); 
            while (uncompressed.peek()!=EOF) {
                u_int64_t temp;
                uncompressed >> temp;
                unc_data.push_back(temp);
            }
            uncompressed.close();
        }
        return unc_data[index_in_file[position]+index-offset[position]];
    }


}

//DEA Deserialization
template<typename T>
void CompressCols::DeltaEADecode() {

    std::string delta_fp = "./out/"+this->split_file_name_+".dea/";
    std::ofstream decoded (this->split_file_path_+".dea_dec", std::ifstream::out);
    
    for (int i=0; i<line_num_; i++) decoded<<this->DeltaEAIndexAt<u_int64_t>(i)<<"\n";

    decoded.close();

}