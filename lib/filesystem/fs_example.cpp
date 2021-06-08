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
                Serial.printf("%s <dir>\n", entry->name().c_str());
                recurseList(entry.get());
            }
            else
            {
                Serial.printf("%s <file>\n", entry->name().c_str());                
            }

            entry.reset(file->getNextFileInDir());
        }
    }
}

void testLittleFS() {

    std::unique_ptr<MFile> readeTest(MFSOwner::File("test"));
    std::shared_ptr<MIstream> readerStream(readeTest->inputStream());
    StreamReader reader(readerStream.get());


    std::string line = reader.readLn();

    while(!reader.eof()) {
        Serial.printf("FSTEST: line read:'%s'\n",line.c_str());
        line = reader.readLn();
    };

    std::unique_ptr<MFile> fileInSub(MFSOwner::File(".sys/mfile_subtest.txt"));

    return;

    Serial.println("FSTEST: test MFile factory");

    std::unique_ptr<MFile> fileInRoot(MFSOwner::File("mfile_test.txt"));
    std::unique_ptr<MFile> aDir(MFSOwner::File(".sys"));

    //std::unique_ptr<MFile> root(MFSOwner::File("dupa/"));

    if(fileInRoot==nullptr) {
        Serial.println("FSTEST: null path returned!!!");
    }

    char exampleText[]="Proletariusze wszystkich krajow, laczcie sie!";

    // test 1 - write some string to a plain file in root dir

    if(fileInRoot->exists()) {
        bool result = fileInRoot->remove();
        Serial.printf("FSTEST: %s existed, delete reult: %d\n", fileInRoot->path().c_str(), result);
    }

    if(fileInSub->exists()) {
        bool result = fileInSub->remove();
        Serial.printf("FSTEST: %s existed, delete reult: %d\n", fileInSub->path().c_str(), result);
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

    Serial.printf("FSTEST: %s = %d\n", aDir->path().c_str(), aDir->isDirectory());

    Serial.printf("FSTEST: %s = %d\n", fileInSub->path().c_str(), fileInSub->isDirectory());

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

    //recurseList(root.get());


    std::unique_ptr<MFile> someComplexPath(MFSOwner::File("http://server.com/dire/ctory/c64archive.7z/games/disc1.dnp/onefile/brucelee.prg"));
    
    Serial.println("FSTEST: dnp container");

    std::unique_ptr<MFile> someComplexPathContainer(MFSOwner::File("http://server.com/dire/ctory/c64archive.7z/games/disc1.dnp"));

    Serial.println("FSTEST: normal file");

}

