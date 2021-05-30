// #ifndef MEATFILE_DEFINES_SMBFS_H
// #define MEATFILE_DEFINES_SMBFS_H

// #include "meat_file.h"

// class SMBFileSystem: public MFileSystem
// {
//     MFile* file(String path) override;
// };

// class SMBFile: public MFile
// {
// public:
//     SMBFile(String path) : MFile(path) {};

//     bool isFile() override;
//     bool isDirectory() override;
//     MIstream* inputStream() override ; // has to return OPENED stream
//     MOstream* outputStream() override ; // has to return OPENED stream
//     time_t getLastWrite() override ;
//     time_t getCreationTime() override ;
//     void setTimeCallback(time_t (*cb)(void)) override ;
//     bool rewindDirectory() override ;
//     MFile* getNextFileInDir() override ;
//     bool mkDir() override ;
//     bool mkDirs() override ;
// };


// #endif