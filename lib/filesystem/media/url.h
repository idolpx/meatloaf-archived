#ifndef MEATFILE_DEFINES_URLFILE_H
#define MEATFILE_DEFINES_URLFILE_H

#include "meat_io.h"
//#include "EdUrlParser.h"
#include "wrappers/line_reader_writer.h"

/********************************************************
 * File implementations
 ********************************************************/

class URLFile: public MFile {
public:
    URLFile(std::string path) : MFile(path) {};

    MIStream* createIStream(MIStream* src) override { return 0; };
    MFile* cd(std::string newDir) override ;
    bool isDirectory() override { return true; };
    MIStream* inputStream() override { return 0; }; // has to return OPENED stream
    MOStream* outputStream() override { return 0; }; // has to return OPENED stream
    time_t getLastWrite() override { return 0; };
    time_t getCreationTime() override { return 0; };
    bool rewindDirectory() { return true; } ;
    MFile* getNextFileInDir() override { return 0; };
    bool mkDir() override { return false; };
    bool exists() override { return true; };
    size_t size() override { return 0; };
    bool remove() override { return false; };
    bool rename(const char* dest) { return false; };

private:
    std::unique_ptr<MFile> pointedFile;

    MFile* getPointed();   
};


/********************************************************
 * FS implementations
 ********************************************************/

class URLFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) override {
        Debug_printv("URLFile [%s]", path.c_str());
        return new URLFile(path); // causes  undefined reference to `vtable for URLFile' WTF?!
    }

public:
    URLFileSystem(): MFileSystem("URL"){}


    bool handles(std::string fileName) {
        //Debug_printv("handles w URL %s %d\n", fileName.rfind(".URL"), fileName.length()-4);
        return byExtension(".url", fileName);
    }


private:
};

#endif