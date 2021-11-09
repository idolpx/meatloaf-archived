#include "d81.h"

// D81 Utility Functions

MIStream* D81File::createIStream(std::shared_ptr<MIStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new D81IStream(containerIstream);
}
