
#include "dnp.h"

/********************************************************
 * File implementations
 ********************************************************/

MIStream* DNPFile::createIStream(std::shared_ptr<MIStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new DNPIStream(containerIstream);
}
