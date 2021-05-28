#include "meat_file.h"

void test() {
    MFile* someFile = MFSOwner::File("meat://c64.meatloaf.cc");
    // do something stupid with it
    if(someFile != nullptr && someFile->isDirectory() && someFile->rewindDirectory()) {
        MFile* file = someFile->getNextFileInDir();
        while(file != nullptr) {
            auto istream = file->inputStream();
            istream->read();
            istream->close();
            delete file;
            file = someFile->getNextFileInDir();
            delete istream;
        };

        if(file != nullptr)
            delete file;

        delete someFile;
    }


    // these files will be of type MFileD64, not HTTP or SMB, but it will handle the image 
    // as if it was on local FS. Of course that requires D64 handling classes to
    // work on MIstream an MFile! And that's also the reason why filesystems have to be 
    // properly ordered in MFSOwner::availableFS
    //
    MFile* someOtherFile2 = MFSOwner::File("smb://192.168.1.10/warez/cyberpunk.d64"); // a D64File instance
    MFile* someOtherFile = MFSOwner::File("http://c64net.org/warez/cyberpunk.d64"); // a D64File instance

    MFile* httpFile = MFSOwner::File("http://meatball.c64.org/newfiles/elite.prg"); // a HTTPFile instance
    MFile* localFile = MFSOwner::File("/downloads/elite.prg"); // a LittleFS instance

    MIstream* istream = httpFile->inputStream();
    MOstream* ostream = localFile->outputStream();

    //ostream.write(istream.readBytes(some_buffer, size));

    delete istream;
    delete ostream;
}

