#include "d8b.h"

// D8B Utility Functions

MIStream* D8BFile::createIStream(std::shared_ptr<MIStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new D8BIStream(containerIstream);
}
