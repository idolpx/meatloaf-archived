#ifndef MEATFILE_DEFAULT_IMPL_H
#define MEATFILE_DEFAULT_IMPL_H

#include "meat_io.h"

class MFileDefaultImpl: public MFile {
    MFileDefaultImpl(std::string path): MFile(path) {};

    MIstream* inputStream() {
        std::shared_ptr<MFile> srcFile(MFSOwner::File(this->streamPath));
        std::shared_ptr<MIstream> srcStream(srcFile->inputStream());

        // if skippable, the skip to srcFile->pathInStream

        return srcStream.get();
    }
};


class MFileSystemDefaultImpl: public MFileSystem {

    bool mount() override { return true; };
    bool umount() override { return true; };

public:
    MFileSystemDefaultImpl(char* prefix) : MFileSystem(prefix){}

    ~MFileSystemDefaultImpl()
    {
        umount();
    }

};

#endif