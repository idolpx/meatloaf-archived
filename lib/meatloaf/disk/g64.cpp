#include "g64.h"

/********************************************************
 * File implementations
 ********************************************************/

MStream* G64File::createIStream(std::shared_ptr<MStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new G64IStream(containerIstream);
}
