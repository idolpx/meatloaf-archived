#ifndef MEATFILE_DEFINES_FSHTTP_H
#define MEATFILE_DEFINES_FSHTTP_H

#include "meat_io.h"
#include "../../include/make_unique.h"
#include "urlfile.h"


class HttpIStream: public MIstream {
public:
    HttpIStream(std::string& path) {
        m_path = path;
    }
    // MStream methods
    bool seek(uint32_t pos, SeekMode mode) override;
    bool seek(uint32_t pos) override;
    size_t position() override;
    void close() override;
    bool open() override;
    ~HttpIStream() {
        close();
    }

    // MIstream methods
    int available() override;
    uint8_t read() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen();

protected:
    std::string m_path;
};


class HttpOStream: public MOstream {
public:
    // MStream methods
    HttpOStream(std::string& path) {
        m_path = path;
    }
    bool seek(uint32_t pos, SeekMode mode) override;
    bool seek(uint32_t pos) override;
    size_t position() override;
    void close() override;
    bool open() override;
    ~HttpOStream() {
        close();
    }

    // MOstream methods
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buf, size_t size) override;
    void flush() override;
    bool isOpen();

protected:
    std::string m_path;
};

class HttpFile: public UrlFile {

public:
    HttpFile(std::string path): UrlFile(path) {};

    bool isDirectory() override;
    MIstream* inputStream() override ; // has to return OPENED stream
    MOstream* outputStream() override ; // has to return OPENED stream
    time_t getLastWrite() override ;
    time_t getCreationTime() override ;
    bool rewindDirectory() override { return false; };
    MFile* getNextFileInDir() override { return nullptr; };
    bool mkDir() override { return false; };
    bool exists() override ; // we may try open the stream to check if it exists
    size_t size() override ; // we may take content-lenght from header if exists
    bool remove() override { return false; };
    bool rename(const char* dest) { return false; };

};


class HttpFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) override {
        return new HttpFile(path);
    }
    bool mount() override {
        return true;
    };
    bool umount() override {
        return true;
    };

    bool handles(std::string path) {
        //Serial.println("FSTEST: handles in http");

        return path.rfind("http://", 0) == 0;
    }
public:
    HttpFileSystem(char* prefix): MFileSystem(prefix) {};
};


#endif