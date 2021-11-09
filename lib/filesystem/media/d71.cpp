#include "d71.h"

// D71 Utility Functions

MIStream* D71File::createIStream(std::shared_ptr<MIStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new D71IStream(containerIstream);
}
