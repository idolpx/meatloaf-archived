#ifndef MEATFILE_DEFINES_URLFILE_H
#define MEATFILE_DEFINES_URLFILE_H

#include "meat_io.h"
#include "EdUrlParser.h"
#include "stream_writer.h"
#include "../../include/make_unique.h"


/********************************************************
 * FS implementations
 ********************************************************/

class URLFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) {
        //return new URLFile(path); // causes  undefined reference to `vtable for URLFile' WTF?!
    };


public:
    URLFileSystem(): MFileSystem("URL"){}


    bool handles(std::string fileName) {
        //Serial.printf("handles w URL %s %d\n", fileName.rfind(".URL"), fileName.length()-4);
        return byExtension(".URL", fileName);
    }


private:
};


/********************************************************
 * File implementations
 ********************************************************/

class UrlFile : public MFile {
public:
    UrlFile(std::string path) : MFile(path) {};
    MIstream* createIStream(MIstream* src) override;
    bool isDirectory() override { return true; };
    bool rewindDirectory() { return true; } ;
    MFile* getNextFileInDir() override ;
    bool mkDir() override { return false; };
    bool exists() override { return true; };

    // all these below are achievable with some trickery
    time_t getLastWrite() override ;
    time_t getCreationTime() override ;
    size_t size() override { return 0; };
    bool remove() override { return false; };
    bool rename(const char* dest) { return false; };

    MFile* cd(std::string newDir) override {
        // cding into urlfile will always just point you to the link read from contents of the file...
        return pointedFile.get();
    }

private:
    std::unique_ptr<MFile> pointedFile;

    MFile* getPointed() {
        if(pointedFile == nullptr) {
            std::unique_ptr<MIstream> istream(inputStream());
            auto reader = std::make_unique<StreamReader>(istream.get());
            auto linkUrl = reader->readLn();
            pointedFile.reset(MFSOwner::File(linkUrl));
        }
        return pointedFile.get();
    }

};

#endif