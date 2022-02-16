#ifndef MEATFILE_DEFINES_7ZIP_H
#define MEATFILE_DEFINES_7ZIP_H

#include "meat_io.h"


/********************************************************
 * Streams implementations
 ********************************************************/

class SevenZipIStream: MIStream {
public:

    SevenZipIStream(MIStream* srcStream): srcStr(srcStream) {
        // this stream must be able to return a stream of
        // UNPACKED file contents
        // additionaly it has to implement getNextEntry()
        // which skips data in the stream to next file in zip
    }
    // MStream methods
    size_t position() override;
    void close() override;
    bool open() override;
    ~SevenZipIStream() {
        close();
    }

    // MIStream methods
    int available() override;
    uint8_t read() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen() override;

protected:
    MStream* srcStr;

};


/********************************************************
 * Files implementations
 ********************************************************/

class SevenZipFile: public MFile
{
public:
    SevenZipFile(std::string path) : MFile(path) {};
    MIStream* createIStream(std::shared_ptr<MIStream> src) override;

    bool isDirectory() override;
    MIStream* inputStream() override ; // has to return OPENED stream
    MOStream* outputStream() override ; // has to return OPENED stream
    time_t getLastWrite() override ;
    time_t getCreationTime() override ;
    bool rewindDirectory() override ;
    MFile* getNextFileInDir() override ;
    bool mkDir() override ;
    bool exists() override ;
    size_t size() override ;
    bool remove() override ;
    bool rename(std::string dest);


    bool isBrowsable() override {
        return true;
    }
};



/********************************************************
 * FS implementations
 ********************************************************/

class SevenZipFileSystem: public MFileSystem
{
    MFile* getFile(std::string path) {
        return new SevenZipFile(path);
    };


public:
    SevenZipFileSystem(): MFileSystem("7z"){}

    bool handles(std::string fileName) {
        return fileName.rfind(".7z") == fileName.length()-4;
    }

private:
};



#endif