#ifndef MEATFILE_DEFINES_SCHEME_CS_H
#define MEATFILE_DEFINES_SCHEME_CS_H

#include "meat_io.h"
#include "WiFiClient.h"
#include "stream_writer.h"

// a scheme for handling Commodore Server
// see: https://www.commodoreserver.com/BlogEntryView.asp?EID=9D133160E7C344A398EC1F45AEF4BF32


class CServerSessionMgr {
    WiFiClient m_wifi;
    std::string m_user;
    std::string m_pass;

public:
    CServerSessionMgr(std::string user = "", std::string pass = "") : m_user(user), m_pass(pass) 
    {
    };
    ~CServerSessionMgr() {
        disconnect();
        if(breader!=nullptr)
            delete breader;
    };
    void connect();
    void disconnect();
    bool command(std::string);
    size_t read(uint8_t* buf, size_t size);
    size_t write(std::string &fileName, const uint8_t *buf, size_t size);
    bool traversePath(MFile* path);
    std::string readReply();
    bool isOK();
    void flush() {
        m_wifi.flush();
    }
    StreamReader* breader;
};

/********************************************************
 * File implementations
 ********************************************************/
class CServerFile: public MFile {
private:
    bool dirIsOpen = false;
    bool dirIsImage = false;
    bool m_isDir;
    size_t m_size;

public:
    CServerFile(std::string path, size_t size = 0): MFile(path) {
        m_size = size;
    };

    bool isDirectory() override;
    MIstream* inputStream() override ; // has to return OPENED stream
    MOstream* outputStream() override ; // has to return OPENED stream
    bool rewindDirectory() override;
    MFile* getNextFileInDir() override;
    bool exists() override ;
    size_t size() override ;
    bool mkDir() override ;
    bool remove() override ;
    MFile* cd(std::string newDir);

    time_t getLastWrite() override { return 0; };
    time_t getCreationTime() override  { return 0; };
    bool rename(const char* dest) { return false; };
    MIstream* createIStream(MIstream* src) { return src; };

};

/********************************************************
 * Streams
 ********************************************************/

//
class CServerIStream: public MIstream {

public:
    CServerIStream(std::string path) {
        url = path;
    }
    ~CServerIStream() {
        close();
    }
    // MStream methods
    bool seek(uint32_t pos, SeekMode mode) override;
    bool seek(uint32_t pos) override;
    size_t position() override;
    void close() override;
    bool open() override;

    // MIstream methods
    int available() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen() override;

protected:
    std::string url;
    bool m_isOpen;
    int m_length;
    int m_bytesAvailable = 0;
    int m_position = 0;
};


class CServerOStream: public MOstream {

public:
    // MStream methods
    CServerOStream(std::string path) {
        url = path;
    }
    ~CServerOStream() {
        close();
    }

    bool seek(uint32_t pos, SeekMode mode) override;
    bool seek(uint32_t pos) override;
    size_t position() override;
    void close() override;
    bool open() override;

    // MOstream methods
    size_t write(const uint8_t *buf, size_t size) override;
    void flush() override;
    bool isOpen() override;

protected:
    std::string url;
    bool m_isOpen;
    WiFiClient m_file;
};


/********************************************************
 * FS
 ********************************************************/

class CServerFileSystem: public MFileSystem 
{
    bool handles(std::string name) {
        return name == "CS:";
    }
public:
    CServerFileSystem(): MFileSystem("c=server") {};
    static CServerSessionMgr session;
    MFile* getFile(std::string path) override {
        return new CServerFile(path);
    }

};


#endif