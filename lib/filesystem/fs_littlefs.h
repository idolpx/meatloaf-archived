#ifndef MEATFILE_DEFINES_FSLITTLE_H
#define MEATFILE_DEFINES_FSLITTLE_H

#include "meat_file.h"

class LittleFileSystem: public MFileSystem 
{
    bool services(String name);
    MFile* create(String path) override;
};

class LittleFile: public MFile
{
public:
    LittleFile(String path) : MFile(path) {};

    bool isFile() override;
    bool isDirectory() override;
    MIstream* inputStream() override ; // has to return OPENED stream
    MOstream* outputStream() override ; // has to return OPENED stream
    time_t getLastWrite() override ;
    time_t getCreationTime() override ;
    void setTimeCallback(time_t (*cb)(void)) override ;
    bool rewindDirectory() override ;
    MFile* getNextFileInDir() override ;
    bool mkDir() override ;
    bool mkDirs() override ;
};

class LittleOStream: public MOstream {
    // MStream methods
    bool seek(uint32_t pos, SeekMode mode) override;
    bool seek(uint32_t pos) override;
    size_t position() const override;
    void close() override;
    bool open() override;
    ~LittleOStream() {
        if(m_isOpen)
            close();
    }

    // MOstream methods
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buf, size_t size) override;
    void flush() override;

};

class LittleIStream: public MIstream {
    // MStream methods
    bool seek(uint32_t pos, SeekMode mode) override;
    bool seek(uint32_t pos) override;
    size_t position() const override;
    void close() override;
    bool open() override;
    ~LittleIStream() {
        if(m_isOpen)
            close();
    }

    // MIstream methods
    int available() override;
    int read() override;
    int peek() override;
    size_t readBytes(char *buffer, size_t length) override;
    size_t read(uint8_t* buf, size_t size) override;
};

#endif