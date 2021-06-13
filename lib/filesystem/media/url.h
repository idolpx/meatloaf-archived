#ifndef MEATFILE_DEFINES_URLFILE_H
#define MEATFILE_DEFINES_URLFILE_H

#include "meat_io.h"
#include "EdUrlParser.h"


/********************************************************
 * FS implementations
 ********************************************************/

class URLFileSystem: public MFileSystem 
{
    MFile* getFile(std::string path) {
        //return new URLFile(path); // causes  undefined reference to `vtable for URLFile' WTF?!
    };


public:
    URLFileSystem(): MFileSystem("URL"){}


    bool handles(std::string fileName) {
        //Serial.printf("handles w URL %s %d\n", fileName.rfind(".URL"), fileName.length()-4);
        return byExtension(".URL", fileName);
    }


private:
};

#endif