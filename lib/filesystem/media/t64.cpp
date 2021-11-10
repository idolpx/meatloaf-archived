#include "t64.h"

// T64 Utility Functions

MIStream* T64File::createIStream(std::shared_ptr<MIStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new T64IStream(containerIstream);
}
