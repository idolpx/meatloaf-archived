#ifndef MEATFILE_DEFINES_URLFILE_H
#define MEATFILE_DEFINES_URLFILE_H

#include "meat_io.h"
//#include "../URLParser/URLParser.h"

class UrlFile: public MFile {
    // incorporate somehow the URLParser
    //URLParser urlParser;

public:
    UrlFile(std::string path): MFile(path) {
        //urlParser.parsePath(path);
    };

};

#endif