// WEBDAV:// - WebDAV

#ifndef MEATLOAF_SCHEME_WEBDAV
#define MEATLOAF_SCHEME_WEBDAV

#include "http.h"

#include "meatloaf.h"
#include "../../include/global_defines.h"


/********************************************************
 * File implementations
 ********************************************************/


class WebDAVFile: public MFile {

public:
    WebDAVFile(std::string path): MFile(path) {};

    bool isDirectory() override;
    MStream* getSourceStream(std::ios_base::openmode mode=std::ios_base::in) override ; // has to return OPENED stream
    time_t getLastWrite() override ;
    time_t getCreationTime() override ;
    bool rewindDirectory() override { return false; };
    MFile* getNextFileInDir() override { return nullptr; };
    bool mkDir() override { return false; };
    bool exists() override ;
    uint32_t size() override ;
    bool remove() override { return false; };
    bool rename(std::string dest) { return false; };
    MStream* getDecodedStream(std::shared_ptr<MStream> src);
    //void addHeader(const String& name, const String& value, bool first = false, bool replace = true);
};


/********************************************************
 * Streams
 ********************************************************/

class WebDAVIOStream: public MStream {
public:
    WebDAVIOStream(std::string& path) {
        url = path;
    }
    ~WebDAVIOStream() {
        close();
    }

    void close() override;
    bool open() override;

    // MStream methods
    uint32_t position() override;
    uint32_t available() override;
    uint32_t read(uint8_t* buf, uint32_t size) override;
    uint32_t write(const uint8_t *buf, uint32_t size) override;
    bool isOpen();

protected:
    std::string url;
    bool _is_open;

//    WiFiClient m_file;
	MeatHttpClient m_http;
};


class WebDAVIStream: public MStream {

public:
    WebDAVIStream(std::string path) {
        //m_http.set_header("user-agent", USER_AGENT);
        //m_http.setUserAgent(USER_AGENT);
        //m_http.setTimeout(10000);
        //m_http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        //m_http.setRedirectLimit(10);
        url = path;
    }
    ~WebDAVIStream() {
        close();
    }

    // MStream methods
    uint32_t size() override;
    uint32_t available() override;     
    uint32_t position() override;

    virtual bool seek(uint32_t pos);

    void close() override;
    bool open() override;

    // MStream methods
    uint32_t read(uint8_t* buf, uint32_t size) override;
    bool isOpen();

protected:
    std::string url;
    bool _is_open;
    bool isFriendlySkipper = false;

//    WiFiClient m_file;
	MeatHttpClient m_http;
};


class WebDAVOStream: public MStream {

public:
    // MStream methods
    WebDAVOStream(std::string path) {
        //m_http.set_header("user-agent", USER_AGENT);
        //m_http.setTimeout(10000);
        //m_http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        //m_http.setRedirectLimit(10);
        //m_http.setReuse(true);

        url = path;
    }
    ~WebDAVOStream() {
        close();
    }

    // MStream methods
    size_t size() override;
    size_t available() override;     
    size_t position() override;

    virtual bool seek(size_t pos);

    void close() override;
    bool open() override;


    // MStream methods
    size_t write(const uint8_t *buf, size_t size) override;
    bool isOpen();

protected:
    std::string url;
    bool _is_open;
    bool isFriendlySkipper = false;
    
//    WiFiClient m_file;
    //WiFiClient m_client;
	MeatHttpClient m_http;
};


/********************************************************
 * FS
 ********************************************************/

class WebDAVFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) override {
        return new WebDAVFile(path);
    }

    bool handles(std::string name) {
        std::string pattern = "webdav:";
        return mstr::equals(name, pattern, false);
    }
public:
    WebDAVFileSystem(): MFileSystem("webdav") {};
};


#endif // MEATLOAF_SCHEME_WEBDAV