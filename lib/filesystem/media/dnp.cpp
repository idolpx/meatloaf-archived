
#include "dnp.h"

DNPFile::DNPFile(std::string path) : MFile(path) {
    media_blocks_free = 65535;
    media_block_size = 1; // blocks are already calculated
    parseUrl(path);
    // Debug_printv("path[%s] size[%d]", path.c_str(), size);
};

MFile* DNPFile::cd(std::string newDir) {
    return 0; 
};