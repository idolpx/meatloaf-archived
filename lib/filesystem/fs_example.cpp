#include "meat_io.h"
#include "../make_unique.h"
#include "fs_littlefs.h"
#include "buffered_io.h"

#define RECORD_SIZE 256

#if defined(ESP32)
uint8_t buffer[RECORD_SIZE] = { 0 };
#else //defined(ESP8266)
char buffer[RECORD_SIZE] = { 0 };
#endif

void testLittleFS() {
    //std::unique_ptr<MFile> fileInRoot(MFSOwner::File("mfile_test.txt"));
    //std::unique_ptr<MFile> fileInSubdir(MFSOwner::File(".sys/mfile_subtest.txt"));

    auto fileInRoot = std::make_unique<LittleFile>("mfile_test.txt");
    auto fileInSub = std::make_unique<LittleFile>(".sys/mfile_subtest.txt");
    auto aDir = std::make_unique<LittleFile>(".sys");

    char exampleText[]="Proletariusze wszystkich krajow, laczcie sie!";

    // test 1 - write some string to a plain file in root dir

    if(fileInRoot->exists()) {
        bool result = fileInRoot->remove();
        Serial.printf("FSTEST: %s existed, delete reult: %d\n", fileInRoot->path(), result);

    }

    if(fileInSub->exists()) {
        bool result = fileInSub->remove();
        Serial.printf("FSTEST: %s existed, delete reult: %d\n", fileInSub->path(), result);
    }


    Serial.println("FSTEST: root file attempt obtain ostream");

    std::unique_ptr<MOstream> writeToSub(fileInSub->outputStream());

    if(writeToSub->isOpen()) {
        Serial.println("FSTEST: root file opened for writing");
        strncpy(buffer, exampleText, RECORD_SIZE);
        Serial.println("FSTEST: root file attempt write");
        int rc = writeToSub->write((uint8_t*)buffer, strlen(exampleText));
        if(rc<=0) {
            Serial.println("FSTEST: Writing root file failed");
        }
        writeToSub->close();
    }
    else {
        Serial.println("FSTEST: Opening fileInRoot for output failed");
    }

    Serial.println("FSTEST: check isDir");

    Serial.printf("FSTEST: %s = %d\n", aDir->path(), aDir->isDirectory());

    Serial.printf("FSTEST: %s = %d\n", fileInSub->path(), fileInSub->isDirectory());

    Serial.println("FSTEST: copying test");

    fileInSub->copyTo(fileInRoot.get());

    // std::shared_ptr<MIstream> readFromSub(fileInSub->inputStream());
    // std::shared_ptr<MOstream> writeToRoot(fileInRoot->outputStream());

    // if(!readFromSub->isOpen()) {
    //     Serial.println("FSTEST: couldn't open a stream for reading");
    // }
    // else if(!writeToRoot->isOpen()) {
    //     Serial.println("FSTEST: couldn't open a stream for writing");
    // }
    // else {
    //     auto br = std::make_unique<BufferedReader>(readFromSub.get());
    //     auto bw = std::make_unique<BufferedWriter>(writeToRoot.get());

    //     //MBuffer* buffer;
    //     do {
    //         auto buffer = br->read();
            
    //         if(buffer->length() != 0) {
    //             Serial.printf("FSTEST: Bytes read into buffred reader: %d\n",buffer->length());
    //             int written = bw->write(buffer);
    //             Serial.printf("FSTEST: Bytes written into buffred writer: %d\n",written);

    //         }
    //     } while (!br->eof());

    //     // int read = readFromSub->read((uint8_t*)buffer, RECORD_SIZE);
    //     // Serial.printf("Read %d bytes\n",read);
    // }

    // readFromSub->close(); // not required, closes automagically
    // writeToRoot->close(); // nor required, closes automagically
}

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

