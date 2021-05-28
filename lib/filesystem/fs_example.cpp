#include "meat_file.h"

void test() {
    MFile someFile = MFSOwner::File("meat://c64.meatloaf.cc");
    if(someFile.isDirectory() && someFile.rewindDirectory()) {
        MFile file = someFile.openNextFile();
        while(file != nullptr) {
            auto istream = file.inputStream();
            istream.read();
            istream.close();
            file = someFile.openNextFile();
        };
    }

    // these files will be of type MFileD64, not HTTP or SMB, but it will handle the image 
    // as if it was on local FS. Of course that requires D64 handling classes to
    // work on MIstream! And that's also the reason why filesystems have to be properly ordered
    // in MFSOwner::availableFS
    //
    MFile someOtherFile = MFSOwner::File("http://c64net.org/warez/cyberpunk.d64");
    MFile someOtherFile2 = MFSOwner::File("smb://192.168.1.10/warez/cyberpunk.d64");

}

