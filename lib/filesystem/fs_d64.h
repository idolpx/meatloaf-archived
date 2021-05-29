// #ifndef MEATFILE_DEFINES_FSD64_H
// #define MEATFILE_DEFINES_FSD64_H

// #include "meat_file.h"

// class D64FileSystem: public MFileSystem 
// {
//     bool services(String name);
//     MFile* create(String path) override;
// };

// class D64File: public MFile
// {
// public:
//     D64File(String path) : MFile(path) {};

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