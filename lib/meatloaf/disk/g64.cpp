#include "g64.h"

/********************************************************
 * File implementations
 ********************************************************/

MIStream* G64File::createIStream(std::shared_ptr<MIStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new G64IStream(containerIstream);
}
