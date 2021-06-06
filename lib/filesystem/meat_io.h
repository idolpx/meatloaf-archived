#ifndef MEATFILE_DEFINES_H
#define MEATFILE_DEFINES_H

#include <memory>
#include <Arduino.h>
#include <string>
#include "FS.h"
#include "buffered_io.h"
#include "meat_stream.h"
#include "../make_unique.h"

#define FS_COUNT 2


/********************************************************
 * Universal file
 ********************************************************/

class MFile {
public:
    MFile(nullptr_t null) : m_isNull(true) {};
    MFile(std::string path);
    MFile(std::string path, std::string name);
    MFile(MFile* path, std::string name);

    std::string name();
    std::string path();
    std::string extension();
    bool operator!=(nullptr_t ptr);

    bool copyTo(MFile* dst) {
        std::unique_ptr<MIstream> istream(this->inputStream());
        std::unique_ptr<MOstream> ostream(dst->outputStream());

        return istream->pipeTo(ostream.get());
    };

    virtual bool isDirectory() = 0;
    virtual MIstream* inputStream() = 0 ; // has to return OPENED stream
    virtual MOstream* outputStream() = 0 ; // has to return OPENED stream
    virtual time_t getLastWrite() = 0 ;
    virtual time_t getCreationTime() = 0 ;
    virtual bool rewindDirectory() = 0 ;
    virtual MFile* getNextFileInDir() = 0 ;
    virtual bool mkDir() = 0 ;
    virtual bool exists() = 0;
    virtual size_t size() = 0;
    virtual bool remove() = 0;
    //virtual bool truncate(size_t size) = 0;
    virtual bool rename(const char* dest) = 0;
    virtual ~MFile() {};
protected:
    std::string m_path;
    bool m_isNull;
};

/********************************************************
 * Filesystem instance
 * it knows how to create a MFile instance!
 ********************************************************/

class MFileSystem {
public:
    MFileSystem(char* prefix);
    virtual bool handles(std::string path) = 0;
    virtual bool mount() = 0;
    virtual bool umount() = 0;
    virtual MFile* getFile(std::string path) = 0;
    bool isMounted() {
        return m_isMounted;
    }

protected:
    char* protocol;
    bool m_isMounted;

    friend class MFSOwner;
};


/********************************************************
 * MFile factory
 ********************************************************/

class MFSOwner {
    static MFileSystem* availableFS[FS_COUNT];

public:
    static MFile* File(std::string name);
    static bool mount(std::string name);
    static bool umount(std::string name);

};

#endif