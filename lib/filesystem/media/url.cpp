
#include "url.h"

MFile* URLFile::cd(std::string newDir) {
    // cding into urlfile will always just point you to the link read from contents of the file...
    Debug_printv("[%s]", newDir.c_str());
    return getPointed();
};


MFile* URLFile::getPointed() {
    Debug_printv("[%s]", path.c_str());
    if(pointedFile == nullptr) {
        std::unique_ptr<MIStream> istream(inputStream());
        auto reader = std::make_unique<LinedReader>(istream.get());
        auto linkUrl = reader->readLn();
        Debug_printv("[%s]", linkUrl.c_str());
        pointedFile.reset(MFSOwner::File(linkUrl));
    }
    return pointedFile.get();
};
