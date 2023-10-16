
// https://stackoverflow.com/questions/22543179/how-to-use-libarchive-properly

#ifndef MEATLOAF_ARCHIVE_7Z
#define MEATLOAF_ARCHIVE_7Z

#include "meat_io.h"

#include <sys/types.h>

#include <sys/stat.h>

#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/********************************************************
 * Streams implementations
 ********************************************************/

class SevenZipIStream: MStream {
public:

    SevenZipIStream(MStream* srcStream): srcStr(srcStream) {
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

    // MStream methods
    size_t available() override;
    //uint8_t read() override;
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
    MStream* createIStream(std::shared_ptr<MStream> src) override;

    bool isDirectory() override;
    MStream* meatStream() override ; // has to return OPENED stream
    time_t getLastWrite() override ;
    time_t getCreationTime() override ;
    bool rewindDirectory() override ;
    MFile* getNextFileInDir() override ;
    bool mkDir() override ;
    bool exists() override ;
    uint32_t size() override ;
    bool remove() override ;
    bool rename(std::string dest);


    // bool isBrowsable() override {
    //     return true;
    // }
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



#endif // MEATLOAF_ARCHIVE_7Z