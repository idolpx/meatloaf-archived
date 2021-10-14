// ML:// - Meatloaf Server Protocol
// 


#ifndef MEATFILE_DEFINES_FSML_H
#define MEATFILE_DEFINES_FSML_H

#if defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#endif

//#include "meat_io.h"
#include "http.h"
#include "../../include/global_defines.h"
#include "helpers.h"
#include "peoples_url_parser.h"

#include <ArduinoJson.h>


/********************************************************
 * File
 ********************************************************/

class MLFile: public HttpFile {

public:
    MLFile(std::string path, size_t size = 0, bool isDir = false): 
    HttpFile(path), m_size(size), m_isDir(isDir)  
    {
        //if(path.back() == '/' || path.find_last_of("/") < 6)
        //    m_isDir = true;

        parseUrl(path);
        //Debug_printv("path[%s] size[%d] is_dir[%d]", path.c_str(), size, isDir);
    };
    ~MLFile();

    bool isDirectory() override { return m_isDir; };
    //void openDir(const char *path) override;
    bool rewindDirectory() override;
    MFile* getNextFileInDir() override;
    MIStream* inputStream() override ; // file on ML server = standard HTTP file available via GET
    
    //MOStream* outputStream() override ; // we can't write to ML server, can we?
    //time_t getLastWrite() override ; // you can implement it if you want
    //time_t getCreationTime() override ; // you can implement it if you want
    //bool mkDir() override { return false; }; // we can't write to ML server, can we?
    //bool exists() override ;
    size_t size() override { return m_size; };
    //bool remove() override { return false; }; // we can't write to ML server, can we?
    //bool rename(std::string dest) { return false; }; // we can't write to ML server, can we?
    //MIStream* createIStream(MIStream* src); // not used anyway

    //std::string mediaRoot();

protected:
    bool dirIsOpen = false;
    String m_lineBuffer;
    WiFiClient m_file;
    HTTPClient m_http;
    StaticJsonDocument<256> m_jsonHTTP;
    size_t m_size = 0;    
    bool m_isDir = false;
};


/********************************************************
 * Streams
 ********************************************************/

class MLIStream: public HttpIStream {

public:
    MLIStream(std::string path) :
    HttpIStream(path)
    {
        m_http.setUserAgent(USER_AGENT);
        m_http.setTimeout(10000);
        m_http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
        m_http.setRedirectLimit(10);
        url = path;
    }
    ~MLIStream() {
        close();
    }

    // MStream methods
    // size_t position() override;
    // void close() override;
    bool open() override;
    

    // MIStream methods
    // int available() override;
    // size_t read(uint8_t* buf, size_t size) override;
    // bool isOpen();

// protected:
//     std::string url;
//     bool m_isOpen;
//     int m_length;
//     WiFiClient m_file;
//     HTTPClient m_http;
//     int m_bytesAvailable = 0;
//     int m_position = 0;
//     bool isFriendlySkipper = false;
};


/********************************************************
 * FS
 ********************************************************/

class MLFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) override {
        // Debug_printv("MLFileSystem::getFile(%s)", path.c_str());
        return new MLFile(path);
    }

    bool handles(std::string name) {
        return name == "ml:";
    }

public:
    MLFileSystem(): MFileSystem("meatloaf") {};
};


#endif