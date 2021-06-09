#ifndef MEATFILE_DEFINES_FSML_H
#define MEATFILE_DEFINES_FSML_H

#include "meat_io.h"
#include "scheme/http.h"
#include "../../include/make_unique.h"
#include "../../include/global_defines.h"
#include "helpers.h"
#include "urlfile.h"
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

class MLFile: public HttpFile {
    bool dirIsOpen = false;
    String m_lineBuffer;
    WiFiClient payload;
    HTTPClient http;
    StaticJsonDocument<256> m_jsonHTTP;
    bool m_isDir;
    size_t m_size;

public:
    MLFile(std::string path, size_t size = 0, bool isDir = false): HttpFile(path), m_isDir(isDir), m_size(size)  {};
    ~MLFile();

    bool isDirectory() override { return m_isDir; };
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
};

#endif