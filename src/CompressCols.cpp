#include <stdio.h>
#include <cstring>
#include <sstream>
#include "succinct_file.h"
#include "lz4.h"
#include "CompressCols.h"
#include "delta_encoded_array.h"
#include <sys/stat.h>


//Class Contructor
CompressCols::CompressCols(std::string file_path, int col_num, bool limit_flag) {
    
    this->ifile_path = file_path;
    this->col_num = col_num;
    this->line_num = 0;

    size_t index {file_path.find_last_of("/", file_path.length())};
    if (index == -1) {
        this->split_file_name = file_path+"_col_"+std::to_string(col_num)+".txt";
        this->split_file_path = "./out/"+this->split_file_name;
    } else {
        this->split_file_name = file_path.substr(index+1, file_path.length())+"_col_"+std::to_string(col_num)+".txt";
        this->split_file_path = "./out/"+this->split_file_name; 
    }
    
    this->split = false;
    this->limit_flag = limit_flag;
    this->compressed = false;
    
}

//Compression function
int CompressCols::Compress(std::string scheme) {
    if (!this->split) this->Split() && std::cout<<"split column "<<this->col_num<<" successfully.\n";

    this->scheme = scheme;

    int return_val = 0;

    switch (this->scheme.c_str()[0]) {
        case 's':
            if (this->scheme!="succinct") break;
            return_val = this->SuccinctCompress();
            this->compressed = true;
            break;
        case 'l':
            if (this->scheme!="lz4") break;
            return_val = this->LZ4Compress();
            this->compressed = true;
            break;
        case 'd':
            if (this->scheme!="dea") break;
            return_val = this->DeltaEAEncode<u_int64_t>();
            this->compressed = true;
            break;
        default:
            break;
    }
    return return_val;
}

//Decompression function (using original compression scheme)
int CompressCols::Decompress() {

    int return_val (0);

    if (!this->compressed) {
        std::cerr<<"File hasn't been compressed yet!\n";
        return return_val;
    }

    switch (this->scheme.c_str()[0]) {
        case 's':
            //return_val = this->SuccinctDecompress();
            break;
        case 'l':
            if (this->scheme!="lz4") break;
            this->LZ4Decompress();
            return_val = 1;
            break;
        case 'd':
            if (this->scheme!="dea") break;
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
int CompressCols::Split() {

    std::string ifp = this->ifile_path;
    std::string sfp = this->split_file_path;

    FILE *fptr = fopen(ifp.c_str(), "r");
    
    if (!fptr) {
        std::cerr<<"SplitToCol: invalid file!\n";
        return 0;
    }
    
    std::ofstream file_out;
    file_out.open(sfp, std::ofstream::trunc);


    int line_count = 0;
    char *buffer = nullptr;
    size_t n = 0;

    while (getline(&buffer, &n, fptr) != -1) {                          //Populating data table

        if (this->limit_flag && line_count>6) break;                    //If limit_flag is set to 1, only six lines will be written to file

        std::string line_read {buffer};                                 //Converting from C string to C++ string for more freedom
        line_read = line_read.substr(0, line_read.find('\n'))+" \n";    //Space added before trailing '\n'
        for (int i =0 ; line_read!="\n"; i++) {                         

            std::string cell_data = line_read.substr(0,
                                            line_read.find(' '));       //This is the string we want to store in the table      
            
            while (cell_data.compare("")==0) {                            //Skip empty strings
                line_read = line_read.substr(line_read.find(' ')+1, 
                                        line_read.size());
                cell_data = line_read.substr(0,
                                            line_read.find(' '));
            }
            
            if(i==this->col_num-1) {                                            //Write to file
                file_out << cell_data << '\n';
                line_count++;              
            }

            line_read = line_read.substr(line_read.find(' ')+1, 
                                        line_read.size());
        }                                                                      
        buffer = nullptr;                                               //Reinitializing buffer and n for getline
        n = 0;
    }

    this->line_num = line_count;
    file_out.close();

    this->split = true;
    return 1;

}


/* ========================
 *   Compression Schemes
 * ========================
 */


/* Succinct */

int CompressCols::SuccinctCompress() {
    std::string sfp = this->split_file_path;
    auto *fd = new SuccinctFile(sfp);

    fd->Serialize(sfp + ".succinct");

    return 1;
}

/*
* LZ4
* 
* Code Source: lz4/examples/blockStreaming_lineByline.c
*/ 

static size_t write_uint16(FILE* fp, uint16_t i) {
    return fwrite(&i, sizeof(i), 1, fp);
}

static size_t write_bin(FILE* fp, const void* array, int arrayBytes)
{
    return fwrite(array, 1, arrayBytes, fp);
}

static size_t read_uint16(FILE* fp, uint16_t* i)
{
    return fread(i, sizeof(*i), 1, fp);
}

static size_t read_bin(FILE* fp, void* array, int arrayBytes)
{
    return fread(array, 1, arrayBytes, fp);
}

//LZ4 Compression function
int CompressCols::LZ4Compress() {
    std::string sfp = this->split_file_path;

    FILE *inpFp = fopen(sfp.c_str(), "rb");
    FILE *outFp = fopen((sfp+".lz4").c_str(), "wb");

    enum {
        messageMaxBytes   = 1024,
        ringBufferBytes   = 1024 * 256 + messageMaxBytes,
    };

    LZ4_stream_t* const lz4Stream = LZ4_createStream();
    const size_t cmpBufBytes = LZ4_COMPRESSBOUND(messageMaxBytes);
    char* const cmpBuf = (char*) malloc(cmpBufBytes);
    char* const inpBuf = (char*) malloc(ringBufferBytes);
    int inpOffset = 0;

    for ( ; ; ) {

        char* const inpPtr = &inpBuf[inpOffset];
        // Read line to the ring buffer.
        int inpBytes = 0;
        if (!fgets(inpPtr, (int) messageMaxBytes, inpFp))
            break;
        inpBytes = (int) strlen(inpPtr);


        {
            const int cmpBytes = LZ4_compress_fast_continue(
                lz4Stream, inpPtr, cmpBuf, inpBytes, cmpBufBytes, 1);
            if (cmpBytes <= 0) break;
            write_uint16(outFp, (uint16_t) cmpBytes);
            write_bin(outFp, cmpBuf, cmpBytes);

            // Add and wraparound the ringbuffer offset
            inpOffset += inpBytes;
            if ((size_t)inpOffset >= ringBufferBytes - messageMaxBytes) inpOffset = 0;
        }
    }
    write_uint16(outFp, 0);

    fclose(inpFp);
    fclose(outFp);
    free(inpBuf);
    free(cmpBuf);
    LZ4_freeStream(lz4Stream);

    return 1;
}

//LZ4 Decompression function
void CompressCols::LZ4Decompress() {
    std::string sfp = this->split_file_path;
    
    FILE *inpFp = fopen((sfp+".lz4").c_str(), "rb");
    FILE *outFp = fopen((sfp+".lz4.dec").c_str(), "wb");

    enum {
        messageMaxBytes   = 1024,
        ringBufferBytes   = 1024 * 256 + messageMaxBytes,
    };

    LZ4_streamDecode_t* const lz4StreamDecode = LZ4_createStreamDecode();
    char* const cmpBuf = (char*) malloc(LZ4_COMPRESSBOUND(messageMaxBytes));
    char* const decBuf = (char*) malloc(ringBufferBytes);
    int decOffset = 0;

    for ( ; ; )
    {
        uint16_t cmpBytes = 0;

        if (read_uint16(inpFp, &cmpBytes) != 1) break;
        if (cmpBytes == 0) break;
        if (read_bin(inpFp, cmpBuf, cmpBytes) != cmpBytes) break;

        {
            char* const decPtr = &decBuf[decOffset];
            const int decBytes = LZ4_decompress_safe_continue(
                lz4StreamDecode, cmpBuf, decPtr, cmpBytes, (int) messageMaxBytes);
            if (decBytes <= 0) break;
            write_bin(outFp, decPtr, decBytes);

            // Add and wraparound the ringbuffer offset
            decOffset += decBytes;
            if ((size_t)decOffset >= ringBufferBytes - messageMaxBytes) decOffset = 0;
        }
    }

    fclose(inpFp);
    fclose(outFp);
    free(decBuf);
    free(cmpBuf);
    LZ4_freeStreamDecode(lz4StreamDecode);
}

/* Delta Endoded Array */

template<typename T>
static T* merge_sort (T* data_array, u_int64_t size) {

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
        T* left = merge_sort(data_array, midpoint+1);
        T* right = merge_sort(data_array+(midpoint+1), size-(midpoint+1));

        u_int64_t l=0; 
        u_int64_t r=0;

        T* sorted = new T[size];

        for (u_int64_t i=0; i<size; i++) {
            if (r >= size-midpoint-1) {sorted[i] = left[l++]; continue;}
            if (l >= midpoint+1) {sorted[i] = right[r++]; continue;}

            if (left[l] < right[r]) sorted[i] = left[l++];
            else sorted[i] = right[r++];
            
        }
        assert(r+l == size);

        return sorted;

    }
}

//DEA_Encoding and Serialization
template<typename T>
int CompressCols::DeltaEAEncode() {

    std::string delta_fp = "./out/"+this->split_file_name+".dea/";
    if(mkdir(delta_fp.c_str(), 0777) == -1) return 0;

    std::ifstream file_in (this->split_file_path);                                                //Reading split file
    std::string read_num;
    T *data_array = new T[this->line_num];                                   
    for (uint64_t i=0; i<line_num && (file_in >> read_num); i++) {                           //Converting strings to T and storing in array

        std::istringstream read_stream (read_num);
        read_stream >> data_array[i];

    }
    file_in.close();

    //Sort
    T* sorted_data = merge_sort<T>(data_array, line_num);
    delete[] data_array;
    data_array = sorted_data;


    std::ofstream metadata (delta_fp+"metadata", std::ofstream::out);
    std::ofstream dea (delta_fp+this->split_file_name+".dea");
    std::ofstream run (delta_fp+this->split_file_name+".run");
    std::ofstream uncompressed (delta_fp+this->split_file_name+".unc");

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
            if (i<this->line_num-1 && data_array[i] < data_array[i+1]) {
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

    for (u_int64_t i=0; i<this->line_num; i++) {
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

            if (i==this->line_num-1) {
                if (in_run) {
                    run_end();
                } else {
                    if (len<run_threshold) unc_end();
                    else dea_end();
                } 
            }
        }
    }
        
    // metadata<<std::to_string(this->line_num)<<"\n";                                 //Sentry
    // metadata<<"-1";

    uncompressed.close();
    run.close();
    dea.close();
    metadata.close();
    
    return 1;
}

static int64_t binary_search (std::vector<u_int64_t> offset, u_int64_t index) {
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

    std::string delta_fp = "./out/"+this->split_file_name+".dea/";
      

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

    int64_t position = binary_search(offset, index);
    assert(position>=0);

    static std::vector<bitmap::EliasGammaDeltaEncodedArray<T>*> dec_arrays {};
    static std::vector<u_int64_t> runs {};
    static std::vector<u_int64_t> unc_data {};

    if (type[position]==0) {
        
        if (dec_arrays.size()==0){
            std::ifstream dea (delta_fp+this->split_file_name+".dea");
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
            std::ifstream run (delta_fp+this->split_file_name+".run");
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
            std::ifstream uncompressed (delta_fp+this->split_file_name+".unc"); 
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

    std::string delta_fp = "./out/"+this->split_file_name+".dea/";
    std::ofstream decoded (this->split_file_path+".dea_dec", std::ifstream::out);
    
    for (int i=0; i<this->line_num; i++) decoded<<this->DeltaEAIndexAt<u_int64_t>(i)<<"\n";

    decoded.close();

}