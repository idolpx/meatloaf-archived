#ifndef MEATFILE_DEFINES_7ZIP_H
#define MEATFILE_DEFINES_7ZIP_H

#include "meat_io.h"


/********************************************************
 * Streams implementations
 ********************************************************/

class SevenZipIStream: MIstream, Browsable {
public:
    SevenZipIStream(std::string& path) {
        m_path = path;
    }
    // MStream methods
    bool seek(uint32_t pos, SeekMode mode) override;
    bool seek(uint32_t pos) override;
    size_t position() override;
    void close() override;
    bool open() override;
    ~SevenZipIStream() {
        close();
    }

    // MIstream methods
    int available() override;
    uint8_t read() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen();

    // Zip-specific methods
    MFile* getNextEntry() override; // skips the stream until the beginnin of next file

protected:
    std::string m_path;

};


/********************************************************
 * Files implementations
 ********************************************************/


/********************************************************
 * FS implementations
 ********************************************************/

class SevenZipFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) {
        return new LittleFile(path);
    };


public:
    SevenZipFileSystem(): MFileSystem("7z"){}

    bool handles(std::string fileName) {
        return fileName.rfind(".7z") == fileName.length()-4;
    }

private:
};



#endif