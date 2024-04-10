#include "d90.h"

/********************************************************
 * File implementations
 ********************************************************/

MIStream* D90File::createIStream(std::shared_ptr<MIStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new D90IStream(containerIstream);
}