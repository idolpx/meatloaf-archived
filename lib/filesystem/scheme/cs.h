// CS:/ - a scheme for handling Commodore Server
// see: https://www.commodoreserver.com/BlogEntryView.asp?EID=9D133160E7C344A398EC1F45AEF4BF32

#ifndef MEATFILESYSTEM_SCHEME_CS
#define MEATFILESYSTEM_SCHEME_CS

#include "../../include/global_defines.h"
#include "../../include/make_unique.h"
#include "meat_io.h"
#include "WiFiClient.h"
#include "wrappers/line_reader_writer.h"
#include "utils.h"
#include "string_utils.h"


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
    LinedReader* breader;
};

/********************************************************
 * File implementations
 ********************************************************/
class CServerFile: public MFile {

public:
    CServerFile(std::string path, size_t size = 0): MFile(path), m_size(size) 
    {
        media_blocks_free = 65535;
        media_block_size = 1; // blocks are already calculated
        parseUrl(path);
        // Debug_printv("path[%s] size[%d]", path.c_str(), size);
    };

    bool isDirectory() override;
    MIStream* inputStream() override ; // has to return OPENED stream
    MOStream* outputStream() override ; // has to return OPENED stream
    bool rewindDirectory() override;
    MFile* getNextFileInDir() override;
    bool exists() override ;
    size_t size() override ;
    bool mkDir() override ;
    bool remove() override ;
    MFile* cd(std::string newDir);

    time_t getLastWrite() override { return 0; };
    time_t getCreationTime() override  { return 0; };
    bool rename(std::string dest) { return false; };
    MIStream* createIStream(MIStream* src) { return src; };

    std::string petsciiName() override {
        return name;
    }


private:
    bool dirIsOpen = false;
    bool dirIsImage = false;
    bool m_isDir;
    size_t m_size;

};

/********************************************************
 * Streams
 ********************************************************/

//
class CServerIStream: public MIStream {

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

    // MIStream methods
    int available() override;
    int size() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen() override;

protected:
    std::string url;
    bool m_isOpen;
    int m_length;
    int m_bytesAvailable = 0;
    int m_position = 0;
};


class CServerOStream: public MOStream {

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

    // MOStream methods
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
        return name == "cs:";
    }
    
public:
    CServerFileSystem(): MFileSystem("c=server") {};
    static CServerSessionMgr session;
    MFile* getFile(std::string path) override {
        return new CServerFile(path);
    }

};


#endif /* MEATFILESYSTEM_SCHEME_CS */
