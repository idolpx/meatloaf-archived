#include "ipfs.h"

#include "../../../include/global_defines.h"
#include "../../../include/debug.h"


MStream* IPFSFile::getSourceStream(std::ios_base::openmode mode) {
    // has to return OPENED stream
    //Debug_printv("[%s]", url.c_str());
    MStream* istream = new IPFSIStream(url);
    istream->open();   
    return istream;
}; 


bool IPFSIStream::open() {
    return _http.GET(url);
};

bool IPFSIStream::seek(uint32_t pos) {
    return true;
}