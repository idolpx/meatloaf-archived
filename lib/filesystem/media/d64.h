#ifndef MEATFILE_DEFINES_D64_H
#define MEATFILE_DEFINES_D64_H

#include "meat_io.h"
#include "LittleFS.h"
#include <string>

/********************************************************
 * Streams implementations
 ********************************************************/

class D64IStream: public MIstream {
public:
    D64IStream(MIstream* srcStream): srcStr(srcStream) {
        // this stream must be able to return a stream of
        // raw file contents from D64 partition
        // additionaly it has to implement getNextEntry()
        // which skips data in the stream to next file in zip
    }
    // MStream methods
    bool seek(uint32_t pos, SeekMode mode) override;
    bool seek(uint32_t pos) override;
    bool seek(uint32_t track, uint32_t sector);
    size_t position() override;
    void close() override;
    bool open() override;
    ~D64IStream() {
        close();
    }

    // MIstream methods
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
    MIstream* createIStream(MIstream* src) override;

    bool isDirectory() override;
    MIstream* inputStream() override ; // has to return OPENED stream
    MOstream* outputStream() override ; // has to return OPENED stream
    time_t getLastWrite() override ;
    time_t getCreationTime() override ;
    bool rewindDirectory() override ;
    MFile* getNextFileInDir() override ;
    bool mkDir() override ;
    bool exists() override ;
    size_t size() override ;
    bool remove() override ;
    bool rename(const char* dest);


};


/********************************************************
 * FS implementations
 ********************************************************/

class D64FileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) {
        //return new D64File(path); // causes  undefined reference to `vtable for D64File' WTF?!
    };


public:
    D64FileSystem(): MFileSystem("d64"){}


    bool handles(std::string fileName) {
        //Serial.printf("handles w D64 %s %d\n", fileName.rfind(".D64"), fileName.length()-4);
        return byExtension(".D64", fileName);
    }


private:
};

#endif