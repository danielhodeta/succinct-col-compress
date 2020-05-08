#include <stdio.h>
#include <cstring>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "succinct_file.h"
#include "lz4frame.h"
#include "CompressCols.h"


//Class Contructor
CompressCols::CompressCols(std::string file_path, int col_num, bool limit_flag) {
    
    this->ifile_path = file_path;
    this->col_num = col_num;

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
            this->LZ4Decompress();
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
    std::string ofp = this->ofile_path;

    FILE * const f_in = fopen(ofp.c_str(), "rb");
    FILE * const f_out = fopen((ofp+".lz4").c_str(), "wb");

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