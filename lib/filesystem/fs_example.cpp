
#include "fs_example.h"

#include "meat_io.h"
#include "../../include/make_unique.h"
#include "buffered_io.h"
#include <string>
#include "../EdUrlParser/EdUrlParser.h"
#include "stream_writer.h"

void testReader(MFile* readeTest) {
    // /* Test Line reader */
    std::shared_ptr<MIstream> readerStream(readeTest->inputStream());
    StreamReader reader(readerStream.get());

    std::string line = reader.readLn();

    while(!reader.eof()) {
        Serial.printf("FSTEST: line read:'%s'\n",line.c_str());
        line = reader.readLn();
    };
}

void testFileProperties(MFile* cserverFile) {
    Serial.printf("FSTEST: %s, isDir = %d\n", cserverFile->path.c_str(), cserverFile->isDirectory());
    Serial.printf("URL: [%s]\n", cserverFile->url().c_str());
    Serial.printf("Root: [%s]\n", cserverFile->root().c_str());
    Serial.printf("Base: [%s]\n", cserverFile->base().c_str());
    Serial.printf("Scheme: [%s]\n", cserverFile->scheme.c_str());
    Serial.printf("Username: [%s]\n", cserverFile->username.c_str());
    Serial.printf("Password: [%s]\n", cserverFile->password.c_str());
    Serial.printf("Host: [%s]\n", cserverFile->hostname.c_str());
    Serial.printf("Port: [%s]\n", cserverFile->port.c_str());
    Serial.printf("Path: [%s]\n", cserverFile->path.c_str());
    Serial.printf("File: [%s]\n", cserverFile->filename.c_str());
    Serial.printf("Extension: [%s]\n", cserverFile->extension.c_str());
    Serial.printf("Query: [%s]\n", cserverFile->query.c_str());
    Serial.printf("Fragment: [%s]\n\n", cserverFile->fragment.c_str());
    Serial.printf("-------------------------------\n");
}

void testDirectory(MFile* dir) {
    std::unique_ptr<MFile> entry(dir->getNextFileInDir());

    while(entry != nullptr) {
        testFileProperties(entry.get());
        entry.reset(dir->getNextFileInDir());
    }
}


void testRecursiveDir(MFile* file) {
    if(file->isDirectory()) {
        std::unique_ptr<MFile> entry(file->getNextFileInDir());

        while(entry != nullptr) {
            if(entry->isDirectory())
            {
                Serial.printf("%s <dir>\n", entry->filename.c_str());
                testRecursiveDir(entry.get());
            }
            else
            {
                Serial.printf("%s <file>\n", entry->filename.c_str());                
            }

            entry.reset(file->getNextFileInDir());
        }
    }
}

void testCreateAndCopy(MFile* fileInRoot, MFile* fileInSub) {
    char exampleText[]="Proletariusze wszystkich krajow, laczcie sie!";

    // test 1 - write some string to a plain file in root dir

    if(fileInRoot->exists()) {
        bool result = fileInRoot->remove();
        Serial.printf("FSTEST: %s existed, delete reult: %d\n", fileInRoot->path.c_str(), result);
    }

    if(fileInSub->exists()) {
        bool result = fileInSub->remove();
        Serial.printf("FSTEST: %s existed, delete reult: %d\n", fileInSub->path.c_str(), result);
    }

    Serial.println("FSTEST: root file attempt obtain ostream");


    std::shared_ptr<MIstream> readFromSub(fileInSub->inputStream());
    std::shared_ptr<MOstream> writeToRoot(fileInRoot->outputStream());

    if(!readFromSub->isOpen()) {
        Serial.println("FSTEST: couldn't open a stream for reading");
    }
    else if(!writeToRoot->isOpen()) {
        Serial.println("FSTEST: couldn't open a stream for writing");
    }
    else {
        auto br = std::make_unique<BufferedReader>(readFromSub.get());
        auto bw = std::make_unique<BufferedWriter>(writeToRoot.get());

        do {
            auto buffer = br->read();
            
            if(buffer->length() != 0) {
                Serial.printf("FSTEST: Bytes read into buffred reader: %d\n",buffer->length());
                int written = bw->write(buffer);
                Serial.printf("FSTEST: Bytes written into buffred writer: %d\n",written);

            }
        } while (!br->eof());
    }

    readFromSub->close(); // not required, closes automagically
    writeToRoot->close(); // nor required, closes automagically
}

void runTests() {

    // this is a directory for verious directories tests
    std::unique_ptr<MFile> testDir(MFSOwner::File("cs:///"));

    // this is a file for tests that run on files
    std::unique_ptr<MFile> testFile(MFSOwner::File("cs:///utilities/disk tools/cie.d64/CIE+SERIAL"));

    // this is a file for write tests
    std::unique_ptr<MFile> destFile(MFSOwner::File("testfile"));

    // Uncomment as needed
    //testReader(testFile.get());
    //testFileProperties(testDir.get());
    //testDirectory(testDir.get());
    //testRecursiveDir(testDir.get());
    //testCreateAndCopy(testFile, destFile);
}

