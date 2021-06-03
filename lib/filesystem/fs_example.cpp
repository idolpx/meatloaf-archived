#include "meat_io.h"

void test() {
    std::unique_ptr<MFile> someFile(MFSOwner::File("meat://c64.meatloaf.cc"));
    // do something stupid with it
    if(someFile != nullptr && someFile->isDirectory() && someFile->rewindDirectory()) {
        std::unique_ptr<MFile> file(someFile->getNextFileInDir());
        while(file != nullptr) {
            std::unique_ptr<MIstream> istream(file->inputStream());
            istream->read();
            istream->close();
            file.reset(someFile->getNextFileInDir());
        };
    }


    // these files will be of type MFileD64, not HTTP or SMB, but it will handle the image 
    // as if it was on local FS. Of course that requires D64 handling classes to
    // work on MIstream an MFile! And that's also the reason why filesystems have to be 
    // properly ordered in MFSOwner::availableFS
    //
    std::unique_ptr<MFile> someOtherFile2(MFSOwner::File("smb://192.168.1.10/warez/cyberpunk.d64")); // a D64File instance
    std::unique_ptr<MFile> someOtherFile(MFSOwner::File("http://c64net.org/warez/cyberpunk.d64")); // a D64File instance

    std::unique_ptr<MFile> httpFile(MFSOwner::File("http://meatball.c64.org/newfiles/elite.prg")); // a HTTPFile instance
    std::unique_ptr<MFile> localFile(MFSOwner::File("/downloads/elite.prg")); // a LittleFS instance

    std::unique_ptr<MIstream> istream(httpFile->inputStream());
    std::unique_ptr<MOstream> ostream(localFile->outputStream());

    //ostream.write(istream.readBytes(some_buffer, size));

}

