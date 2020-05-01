#include <stdio.h>
#include <cstring>
#include "lz4.h"
#include "lz4_compress.h"

//Source: lz4/examples/blockStreaming_lineByline.c
static size_t write_uint16(FILE* fp, uint16_t i) {
    return fwrite(&i, sizeof(i), 1, fp);
}

//Source: lz4/examples/blockStreaming_lineByline.c
static size_t write_bin(FILE* fp, const void* array, int arrayBytes)
{
    return fwrite(array, 1, arrayBytes, fp);
}
//Source: lz4/examples/blockStreaming_lineByline.c
static size_t read_uint16(FILE* fp, uint16_t* i)
{
    return fread(i, sizeof(*i), 1, fp);
}
//Source: lz4/examples/blockStreaming_lineByline.c
static size_t read_bin(FILE* fp, void* array, int arrayBytes)
{
    return fread(array, 1, arrayBytes, fp);
}

//Source: lz4/examples/blockStreaming_lineByline.c
int LZ4Compress(std::string file_name) {

    FILE *inpFp = fopen(file_name.c_str(), "rb");
    FILE *outFp = fopen((file_name+".lz4").c_str(), "wb");

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
}

//Source: lz4/examples/blockStreaming_lineByline.c
void LZ4Decompress(std::string file_name) {
    
    FILE *inpFp = fopen((file_name+".lz4").c_str(), "rb");
    FILE *outFp = fopen((file_name+".lz4.dec").c_str(), "wb");

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