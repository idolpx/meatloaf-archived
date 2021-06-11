#include "meat_io.h"
#include "../../include/make_unique.h"
#include "buffered_io.h"
#include <string>
#include "../EdUrlParser/EdUrlParser.h"
#include "stream_writer.h"

#define RECORD_SIZE 256

#if defined(ESP32)
uint8_t buffer[RECORD_SIZE] = { 0 };
#else //defined(ESP8266)
char buffer[RECORD_SIZE] = { 0 };
#endif

void recurseList(MFile* file) {
    if(file->isDirectory()) {
        std::unique_ptr<MFile> entry(file->getNextFileInDir());

        while(entry != nullptr) {
            if(entry->isDirectory())
            {
                Serial.printf("%s <dir>\n", entry->filename.c_str());
                recurseList(entry.get());
            }
            else
            {
                Serial.printf("%s <file>\n", entry->filename.c_str());                
            }

            entry.reset(file->getNextFileInDir());
        }
    }
}

void testLittleFS() {


    // /* Test Line reader */
    // std::unique_ptr<MFile> readeTest(MFSOwner::File("test"));
    // std::shared_ptr<MIstream> readerStream(readeTest->inputStream());
    // StreamReader reader(readerStream.get());


    // std::string line = reader.readLn();

    // while(!reader.eof()) {
    //     Serial.printf("FSTEST: line read:'%s'\n",line.c_str());
    //     line = reader.readLn();
    // };


    // return;

    // std::unique_ptr<MFile> fileInSub(MFSOwner::File(".sys/mfile_subtest.txt"));

    Serial.println("FSTEST: test MFile factory");

    // std::unique_ptr<MFile> fileInRoot(MFSOwner::File("mfile_test.txt"));
    // std::unique_ptr<MFile> aDir(MFSOwner::File(".sys"));

    // std::unique_ptr<MFile> cserverFile(MFSOwner::File("cs://utilities/disk tools/cie.d64/cie+serial"));
    // std::shared_ptr<MIstream> cserverStream(cserverFile->inputStream());

    std::unique_ptr<MFile> cserverPath(MFSOwner::File("cs://utilities/disk tools"));
    cserverPath->rewindDirectory();

    // std::unique_ptr<MFile> complexPath(MFSOwner::File("ml://c64.meatloaf.cc/some/deeper/d64/path.zip/file/inside/andextension.sid"));

    // std::unique_ptr<MFile> urlFile(MFSOwner::File("ml://c64.meatloaf.cc/"));

    // Serial.printf("URL: [%s]\n", complexPath->url.c_str());
    // Serial.printf("Root: [%s]\n", complexPath->root.c_str());
    // Serial.printf("Base: [%s]\n", complexPath->base.c_str());
    // Serial.printf("Scheme: [%s]\n", complexPath->scheme.c_str());
    // Serial.printf("Username: [%s]\n", complexPath->username.c_str());
    // Serial.printf("Password: [%s]\n", complexPath->password.c_str());
    // Serial.printf("Host: [%s]\n", complexPath->hostname.c_str());
    // Serial.printf("Port: [%s]\n", complexPath->port.c_str());
    // Serial.printf("Path: [%s]\n", complexPath->pathX.c_str());
    // Serial.printf("File: [%s]\n", complexPath->filename.c_str());
    // Serial.printf("Extension: [%s]\n", complexPath->extension.c_str());
    // Serial.printf("Query: [%s]\n", complexPath->query.c_str());
    // Serial.printf("Fragment: [%s]\n\n", complexPath->fragment.c_str());
    // Serial.printf("MFile path: [%s]\n", complexPath->path().c_str());
    // Serial.printf("-------------------------------\n");

    std::unique_ptr<MFile> entry(cserverPath->getNextFileInDir());

    while(entry != nullptr) {
        entry.reset(cserverPath->getNextFileInDir());
    }

    // if(urlFile==nullptr) {
    //     Serial.println("FSTEST: null path returned!!!");
    // } else {
    //     Serial.printf("FSTEST: url %s exists =%d\n", urlFile->path().c_str(), urlFile->exists());
    //     Serial.printf("FSTEST: url %s size =%d\n", urlFile->path().c_str(), urlFile->size());

    //     std::unique_ptr<MIstream> urlistream(urlFile->inputStream());
    //     StreamReader reader(urlistream.get());

    //     std::string line = reader.readLn();

    //     while(!reader.eof()) {
    //         Serial.printf("FSTEST: line read:'%s'\n",line.c_str());
    //         line = reader.readLn();
    //     };

    // }
    // return;

    // char exampleText[]="Proletariusze wszystkich krajow, laczcie sie!";

    // // test 1 - write some string to a plain file in root dir

    // if(fileInRoot->exists()) {
    //     bool result = fileInRoot->remove();
    //     Serial.printf("FSTEST: %s existed, delete reult: %d\n", fileInRoot->path().c_str(), result);
    // }

    // if(fileInSub->exists()) {
    //     bool result = fileInSub->remove();
    //     Serial.printf("FSTEST: %s existed, delete reult: %d\n", fileInSub->path().c_str(), result);
    // }


    // Serial.println("FSTEST: root file attempt obtain ostream");

    // std::unique_ptr<MOstream> writeToSub(fileInSub->outputStream());

    // if(writeToSub->isOpen()) {
    //     Serial.println("FSTEST: root file opened for writing");
    //     strncpy(buffer, exampleText, RECORD_SIZE);
    //     Serial.println("FSTEST: root file attempt write");
    //     int rc = writeToSub->write((uint8_t*)buffer, strlen(exampleText));
    //     if(rc<=0) {
    //         Serial.println("FSTEST: Writing root file failed");
    //     }
    //     writeToSub->close();
    // }
    // else {
    //     Serial.println("FSTEST: Opening fileInRoot for output failed");
    // }

    // Serial.println("FSTEST: check isDir");

    // Serial.printf("FSTEST: %s = %d\n", aDir->path().c_str(), aDir->isDirectory());

    // Serial.printf("FSTEST: %s = %d\n", fileInSub->path().c_str(), fileInSub->isDirectory());

    // Serial.println("FSTEST: copying test");

    // fileInSub->copyTo(fileInRoot.get());

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

    //recurseList(root.get());


    // std::unique_ptr<MFile> someComplexPath(MFSOwner::File("http://server.com/dire/ctory/c64archive.7z/games/disc1.dnp/onefile/brucelee.prg"));
    
    // Serial.println("FSTEST: dnp container");

    // std::unique_ptr<MFile> someComplexPathContainer(MFSOwner::File("http://server.com/dire/ctory/c64archive.7z/games/disc1.dnp"));

    // Serial.println("FSTEST: normal file");

}

