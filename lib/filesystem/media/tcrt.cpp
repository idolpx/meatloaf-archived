#include "tcrt.h"

// TCRT Utility Functions

MIStream* TCRTFile::createIStream(std::shared_ptr<MIStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new TCRTIStream(containerIstream);
}
