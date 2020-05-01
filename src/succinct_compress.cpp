#include "succinct_compress.h"
#include "succinct_file.h"

int SuccinctCompress(std::string file_name) {
    auto *fd = new SuccinctFile(file_name);

    fd->Serialize(file_name + ".succinct");

    return 1;
}