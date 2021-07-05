
#include "url.h"

MFile* URLFile::cd(std::string newDir) {
    // cding into urlfile will always just point you to the link read from contents of the file...
    Debug_printv("[%s]", newDir.c_str());
    return getPointed();
};


MFile* URLFile::getPointed() {
    Debug_printv("[%s]", path.c_str());
    if(pointedFile == nullptr) {
    Debug_printv("[%s]", path.c_str());
        std::unique_ptr<MIStream> istream(inputStream());
    Debug_printv("[%s]", path.c_str());
    if(istream == nullptr) {
        Debug_printv("ISTREAM == NULLPTR!!! [%s]", path.c_str());
    }
        auto reader = std::make_unique<LinedReader>(istream.get());
    Debug_printv("[%s]", path.c_str());
        auto linkUrl = reader->readLn();
    Debug_printv("[%s]", path.c_str());
        Debug_printv("[%s]", linkUrl.c_str());
    Debug_printv("[%s]", path.c_str());
        pointedFile.reset(MFSOwner::File(linkUrl));
    }
    Debug_printv("[%s]", path.c_str());
    return pointedFile.get();
};
