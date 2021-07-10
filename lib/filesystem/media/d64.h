#ifndef MEATFILE_DEFINES_D64_H
#define MEATFILE_DEFINES_D64_H

#include "meat_io.h"
#include <string>

/********************************************************
 * Streams implementations
 ********************************************************/

class D64IStream: public MIStream {
public:
    D64IStream(MIStream* srcStream): srcStr(srcStream) {
        // this stream must be able to return a stream of
        // raw file contents from D64 partition
        // additionaly it has to implement getNextEntry()
        // which skips data in the stream to next file in zip
    }
    // MStream methods
    bool seek(uint32_t pos) override;
    bool seek(uint32_t track, uint32_t sector);
    size_t position() override;
    void close() override;
    bool open() override;
    ~D64IStream() {
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
 * Files implementations
 ********************************************************/

class D64File: public MFile {
public:
    D64File(std::string path) : MFile(path) {};
    MIStream* createIStream(MIStream* src) override { return 0; };

    MFile* cd(std::string newDir) override { return 0; };
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
 * FS implementations
 ********************************************************/

class D64FileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) override {
        return new D64File(path); // causes  undefined reference to `vtable for D64File' WTF?!
    };


public:
    D64FileSystem(): MFileSystem("d64"){}


    bool handles(std::string fileName) {
        //Serial.printf("handles w D64 %s %d\n", fileName.rfind(".D64"), fileName.length()-4);
        return byExtension(".d64", fileName);
    }


private:
};

#endif