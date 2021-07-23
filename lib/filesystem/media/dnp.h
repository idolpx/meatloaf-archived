// .DNP - CMD hard Disk Native Partition
// https://ist.uwaterloo.ca/~schepers/formats/D2M-DNP.TXT
//

#ifndef MEATFILE_DEFINES_DNP_H
#define MEATFILE_DEFINES_DNP_H

#include "meat_io.h"
#include <string>

#include "../../include/global_defines.h"


/********************************************************
 * Files implementations
 ********************************************************/

class DNPFile: public MFile {
public:
    DNPFile(std::string path);
    MIStream* createIStream(MIStream* src) override { return 0; };

    MFile* cd(std::string newDir) override;
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
    bool rename(std::string dest) { return false; };

};


/********************************************************
 * Streams implementations
 ********************************************************/
class DNPIStream: public MIStream {
public:
    DNPIStream(MIStream* srcStream): srcStr(srcStream) {
        // this stream must be able to return a stream of
        // raw file contents from DNP partition
        // additionaly it has to implement getNextEntry()
        // which skips data in the stream to next file in zip
    }
    // MStream methods
    size_t position() override;
    void close() override;
    bool open() override;
    ~DNPIStream() {
        close();
    }

    // MIStream methods
    int available() override;
    //uint8_t read() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen();
    bool isBrowsable() override {
        return true;
    }

protected:
    MStream* srcStr;

};


/********************************************************
 * FS implementations
 ********************************************************/

class DNPFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) override {
        return new DNPFile(path); // causes  undefined reference to `vtable for DNPFile' WTF?!
    }


public:
    DNPFileSystem(): MFileSystem("dnp"){}


    bool handles(std::string fileName) {
        //Serial.printf("handles w dnp %s %d\n", fileName.rfind(".dnp"), fileName.length()-4);
        return byExtension(".dnp", fileName);
    }


private:
};

#endif