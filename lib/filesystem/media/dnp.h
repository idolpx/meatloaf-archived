#ifndef MEATFILE_DEFINES_DNP_H
#define MEATFILE_DEFINES_DNP_H

#include "meat_io.h"
#include "LittleFS.h"
#include <string>

/********************************************************
 * Streams implementations
 ********************************************************/

class DNPIStream: public MIstream {
public:
    DNPIStream(MIstream* srcStream): srcStr(srcStream) {
        // this stream must be able to return a stream of
        // raw file contents from DNP partition
        // additionaly it has to implement getNextEntry()
        // which skips data in the stream to next file in zip
    }
    // MStream methods
    bool seek(uint32_t pos, SeekMode mode) override;
    bool seek(uint32_t pos) override;
    size_t position() override;
    void close() override;
    bool open() override;
    ~DNPIStream() {
        close();
    }

    // MIstream methods
    int available() override;
    //uint8_t read() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen();

protected:
    MStream* srcStr;

};

/********************************************************
 * Files implementations
 ********************************************************/

class DnpFile: public MFile {
public:
    DnpFile(std::string path) : MFile(path) {};
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

    bool isBrowsable() override {
        return true;
    }

    MFile* getNextEntry() override; // skips the stream until the beginnin of next file


};


/********************************************************
 * FS implementations
 ********************************************************/

class DNPFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) {
        //return new DnpFile(path); // causes  undefined reference to `vtable for DnpFile' WTF?!
    };


public:
    DNPFileSystem(): MFileSystem("dnp"){}


    bool handles(std::string fileName) {
        //Serial.printf("handles w dnp %s %d\n", fileName.rfind(".dnp"), fileName.length()-4);
        return fileName.rfind(".dnp") != -1 &&  fileName.rfind(".dnp") == fileName.length()-4;
    }


private:
};

#endif