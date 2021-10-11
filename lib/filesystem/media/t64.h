// .T64 - The T64 tape image format
// https://vice-emu.sourceforge.io/vice_17.html#SEC331
// https://ist.uwaterloo.ca/~schepers/formats/T64.TXT
//

#ifndef MEATFILE_DEFINES_FST64_H
#define MEATFILE_DEFINES_FST64_H

#include "meat_io.h"
#include "../../include/global_defines.h"


/********************************************************
 * File implementations
 ********************************************************/


class T64File: public MFile {

public:
    T64File(std::string path): MFile(path) {};

    bool isDirectory() override;
    MIStream* inputStream() override ; // has to return OPENED stream
    MOStream* outputStream() override ; // has to return OPENED stream
    time_t getLastWrite() override ;
    time_t getCreationTime() override ;
    bool rewindDirectory() override { return false; };
    MFile* getNextFileInDir() override { return nullptr; };
    bool mkDir() override { return false; };
    bool exists() override ;
    size_t size() override ;
    bool remove() override { return false; };
    bool rename(std::string dest) { return false; };
    MIStream* createIStream(MIStream* src);
    //void addHeader(const String& name, const String& value, bool first = false, bool replace = true);

protected:
    void fillPaths(std::vector<std::string>::iterator* matchedElement, std::vector<std::string>::iterator* fromStart, std::vector<std::string>::iterator* last);
};


/********************************************************
 * Streams
 ********************************************************/

class T64IOStream: public MIStream, MOStream {
public:
    T64IOStream(std::string& path) {
        url = path;
    }
    ~T64IOStream() {
        close();
    }

    void close() override;
    bool open() override;

    // MStream methods
    size_t position() override;
    int available() override;
    size_t read(uint8_t* buf, size_t size) override;
    size_t write(const uint8_t *buf, size_t size) override;
    bool isOpen();

protected:
    std::string url;
    bool m_isOpen;
    int m_length;
    int m_bytesAvailable = 0;
    int m_position = 0;
};


class T64IStream: public MIStream {

public:
    T64IStream(std::string path) {
        m_http.setUserAgent(USER_AGENT);
        m_http.setTimeout(10000);
        m_http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        m_http.setRedirectLimit(10);
        url = path;
    }
    // MStream methods
    size_t position() override;
    void close() override;
    bool open() override;
    ~T64IStream() {
        close();
    }

    // MIStream methods

    virtual bool seek(uint32_t pos);
    int available() override;
    size_t size() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen();

protected:
    std::string url;
    bool m_isOpen;
    int m_bytesAvailable = 0;
    int m_length = 0;
    uint32_t m_position = 0;
    bool isFriendlySkipper = false;
};


class T64OStream: public MOStream {

public:
    // MStream methods
    T64OStream(std::string path) {
        url = path;
    }
    size_t position() override;
    void close() override;
    bool open() override;
    ~T64OStream() {
        close();
    }

    // MOStream methods
    size_t write(const uint8_t *buf, size_t size) override;
    bool isOpen();

protected:
    std::string url;
    bool m_isOpen;
};


/********************************************************
 * FS
 ********************************************************/

class T64FileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) override {
        return new T64File(path);
    }

    bool handles(std::string extension) {
        std::string pattern = "T64";
        return mstr::equals(extension, pattern, false);
    }
public:
    T64FileSystem(): MFileSystem("t64") {};
};


#endif