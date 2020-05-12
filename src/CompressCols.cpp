#include <stdio.h>
#include <cstring>
#include <sstream>
#include "succinct_file.h"
#include "lz4.h"
#include "CompressCols.h"
#include "delta_encoded_array.h"


//Class Contructor
CompressCols::CompressCols(std::string file_path, int col_num, bool limit_flag) {
    
    this->ifile_path = file_path;
    this->col_num = col_num;
    this->line_num = 0;

    size_t index {file_path.find_last_of("/", file_path.length())};
    if (index == -1)
        this->ofile_path = "./out/"+file_path+"_col_"+std::to_string(col_num)+".txt";
    else
        this->ofile_path = "./out/"+file_path.substr(index+1, file_path.length())+"_col_"+std::to_string(col_num)+".txt"; 

    
    this->split = false;
    this->limit_flag = limit_flag;
    this->compressed = false;
    
}

//Compression function
int CompressCols::Compress(std::string scheme) {
    if (!this->split) this->Split();

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
    std::string ofp = this->ofile_path;

    FILE *fptr = fopen(ifp.c_str(), "r");
    
    if (!fptr) {
        std::cerr<<"SplitToCol: invalid file!\n";
        return 0;
    }
    
    std::ofstream file_out;
    file_out.open(ofp, std::ofstream::trunc);


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
    std::string ofp = this->ofile_path;
    auto *fd = new SuccinctFile(ofp);

    fd->Serialize(ofp + ".succinct");

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
    std::string ofp = this->ofile_path;

    FILE *inpFp = fopen(ofp.c_str(), "rb");
    FILE *outFp = fopen((ofp+".lz4").c_str(), "wb");

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
    std::string ofp = this->ofile_path;
    
    FILE *inpFp = fopen((ofp+".lz4").c_str(), "rb");
    FILE *outFp = fopen((ofp+".lz4.dec").c_str(), "wb");

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

//DEA Encoding and Serialization
template<typename data_type>
int CompressCols::DeltaEAEncode() {
    
    u_int64_t ln = this->line_num;
    std::string ofp = this->ofile_path;

    std::ifstream file_in (this->ofile_path);                                                //Reading split file
    std::string read_num;
    data_type *data_array = new data_type[this->line_num];                                   
    for (uint64_t i=0; i<line_num && (file_in >> read_num); i++) {                           //Converting strings to data_type and storing in array

        std::istringstream read_stream (read_num);
        read_stream >> data_array[i];

    }

    bitmap::EliasGammaDeltaEncodedArray<data_type> enc_array(data_array, ln);               //Encoding array

    std::ofstream file_out;
    file_out.open(ofp+".delta", std::ofstream::out);
    enc_array.Serialize(file_out);                                                          //Serializing array
    file_out.close();
    
    return 1;
}

//DEA Deserialization
template<typename data_type>
void CompressCols::DeltaEADecode() {

    std::string ofp = this->ofile_path;

    bitmap::EliasGammaDeltaEncodedArray<data_type> dec_array(NULL, 0);
    std::ifstream file_in;
    file_in.open(ofp+".delta", std::ofstream::in);
    dec_array.Deserialize(file_in);

    std::ofstream file_out;
    file_out.open(ofp+".delta.dec", std::ofstream::out);
    for (uint64_t i=0; i<this->line_num; i++) {                                               //Printing after deserialize
        file_out<<dec_array[i]<<"\n";
    }
    file_in.close();
    file_out.close();

}