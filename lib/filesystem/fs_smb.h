#ifndef MEATFILE_DEFINES_SMBFS_H
#define MEATFILE_DEFINES_SMBFS_H

#include "meat_file.h"

class SMBFileSystem: public MFileSystem
{
    MFile create(String path) override;
};

#endif