// HTTP:// - Hypertext Transfer Protocol

#ifndef MEATLOAF_SCHEME_HTTP
#define MEATLOAF_SCHEME_HTTP

#include "meat_io.h"
#include "../../include/global_defines.h"
#if defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

/********************************************************
 * File implementations
 ********************************************************/


class HttpFile: public MFile {

public:
    HttpFile(std::string path): MFile(path) {};

    bool isDirectory() override;
//    MStream* inputStream() override; // has to return OPENED stream
//    MOStream* outputStream() override ; // has to return OPENED stream
    time_t getLastWrite() override ;
    time_t getCreationTime() override ;
    bool rewindDirectory() override { return false; };
    MFile* getNextFileInDir() override { return nullptr; };
    bool mkDir() override { return false; };
    bool exists() override ;
    size_t size() override ;
    bool remove() override { return false; };
    bool rename(std::string dest) { return false; };
    MStream* createIStream(std::shared_ptr<MStream> src);
    //void addHeader(const String& name, const String& value, bool first = false, bool replace = true);
};


/********************************************************
 * Streams
 ********************************************************/

// class HttpIOStream: public MStream, MOStream {
// public:
//     HttpIOStream(std::string& path) {
//         url = path;
//     }
//     ~HttpIOStream() {
//         close();
//     }

//     void close() override;
//     bool open() override;

//     // MStream methods
//     size_t position() override;
//     size_t available() override;
//     size_t read(uint8_t* buf, size_t size) override;
//     size_t write(const uint8_t *buf, size_t size) override;
//     bool isOpen();

// protected:
//     std::string url;
//     bool m_isOpen;
//     size_t m_length;
//     size_t m_bytesAvailable = 0;
//     size_t m_position = 0;
       
//     WiFiClient m_file;
// 	HTTPClient m_http;
// };


class HttpStream: public MStream {

public:
    HttpStream(std::string path) {
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
    ~HttpStream() {
        close();
    }

    // MStream methods

    virtual bool seek(size_t pos);
    size_t available() override;
    size_t size() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen();

protected:
    std::string url;
    bool m_isOpen;
    size_t m_bytesAvailable = 0;
    size_t m_length = 0;
    size_t m_position = 0;
    bool isFriendlySkipper = false;

    WiFiClient m_file;
	HTTPClient m_http;
};


// class HttpOStream: public MOStream {

// public:
//     // MStream methods
//     HttpOStream(std::string path) {
//         m_http.setUserAgent(USER_AGENT);
//         m_http.setTimeout(10000);
//         m_http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
//         m_http.setRedirectLimit(10);
//         m_http.setReuse(true);

//         url = path;
//     }
//     size_t position() override;
//     void close() override;
//     bool open() override;
//     ~HttpOStream() {
//         close();
//     }

//     // MOStream methods
//     size_t write(const uint8_t *buf, size_t size) override;
//     bool isOpen();

// protected:
//     std::string url;
//     bool m_isOpen;
//     WiFiClient m_file;
//     //WiFiClient m_client;
// 	HTTPClient m_http;
// };


/********************************************************
 * FS
 ********************************************************/

class HttpFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) override {
        return new HttpFile(path);
    }

    bool handles(std::string name) {
        if ( mstr::equals(name, (char *)"http:", false) )
            return true;

        if ( mstr::equals(name, (char *)"https:", false) )
            return true;
            
        return false;
    }
public:
    HttpFileSystem(): MFileSystem("http") {};
};


#endif /* MEATLOAF_SCHEME_HTTP */
