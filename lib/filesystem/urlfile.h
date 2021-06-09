#ifndef MEATFILE_DEFINES_URLFILE_H
#define MEATFILE_DEFINES_URLFILE_H

#include "meat_io.h"
#include "EdUrlParser.h"

class UrlFile: public MFile {

public:
    EdUrlParser urlParser;
    UrlFile(std::string path): MFile(path) {
        urlParser.parsePath(path);
    };

};

#endif