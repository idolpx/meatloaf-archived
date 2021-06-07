#ifndef MEATFILE_DEFINES_DNP_H
#define MEATFILE_DEFINES_DNP_H

#include "meat_io.h"
#include "LittleFS.h"
#include "mfile_default_impl.h"
#include <string>

class DNPFileSystem: public MFileSystemDefaultImpl 
{
    MFile* getFile(std::string path) {
        return new LittleFile(path);
    };


public:
    DNPFileSystem(): MFileSystemDefaultImpl("dnp"){}


    bool handles(std::string fileName) {
        //Serial.printf("handles w dnp %s %d\n", path.c_str(), path.rfind(".dnp"));
        return fileName.rfind(".dnp") == fileName.length()-4;
    }


private:
};

#endif