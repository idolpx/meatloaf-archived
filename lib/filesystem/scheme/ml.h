// ML:// - Meatloaf Server Protocol
// 


#ifndef MEATFILE_DEFINES_FSML_H
#define MEATFILE_DEFINES_FSML_H

#include "meat_io.h"
#include "scheme/http.h"
#include "../../include/global_defines.h"
#include "helpers.h"
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>


/********************************************************
 * File
 ********************************************************/

class MLFile: public HttpFile {
    bool dirIsOpen = false;
    String m_lineBuffer;
    WiFiClient m_file;
	HTTPClient m_http;
    StaticJsonDocument<256> m_jsonHTTP;
    bool m_isDir;
    size_t m_size;

public:
    MLFile(std::string path, size_t size = 0, bool isDir = false): 
    HttpFile(path), m_size(size), m_isDir(isDir)  
    {
        if(path.back() == '/')
            m_isDir = true;

        parseUrl(path);
        Debug_printv("path[%s] size[%d] is_dir[%d]", path.c_str(), size, isDir);
    };
    ~MLFile();

    bool isDirectory() override { return m_isDir; };
    //void openDir(const char *path) override;
    bool rewindDirectory() override;
    MFile* getNextFileInDir() override;
    // MIstream* inputStream() override ; // file on ML server = standard HTTP file available via GET
    // MOstream* outputStream() override ; // we can't write to ML server, can we?
    //time_t getLastWrite() override ; // you can implement it if you want
    //time_t getCreationTime() override ; // you can implement it if you want
    //bool mkDir() override { return false; }; // we can't write to ML server, can we?
    // bool exists() override ;
    size_t size() override { return m_size; };
    //bool remove() override { return false; }; // we can't write to ML server, can we?
    //bool rename(const char* dest) { return false; }; // we can't write to ML server, can we?
    //MIstream* createIStream(MIstream* src); // not used anyway

    //std::string mediaRoot();
};


/********************************************************
 * FS
 ********************************************************/

class MLFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) override {
        Debug_printv("MLFileSystem::getFile(%s)", path.c_str());
        return new MLFile(path);
    }

    bool handles(std::string name) {
        return name == "ML:";
    }
public:
    MLFileSystem(): MFileSystem("meatloaf") {};
};


#endif