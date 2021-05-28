#ifndef MEATFILE_DEFINES_FSD64_H
#define MEATFILE_DEFINES_FSD64_H

#include "meat_file.h"

class D64FileSystem: public MFileSystem 
{
    bool services(String name);
    MFile create(String path) override;
};

class D64File: public MFile
{
public:
    D64File(String path) : MFile(path) {};
};

#endif