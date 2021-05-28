#ifndef MEATFILE_DEFINES_H
#define MEATFILE_DEFINES_H

#include <memory>
#include <Arduino.h>
#include "FS.h"

#define FS_COUNT 2

/********************************************************
 * Universal streams
 ********************************************************/

class MStream 
{
public:
    virtual bool seek(uint32_t pos, SeekMode mode) = 0;
    virtual bool seek(uint32_t pos) = 0;
    virtual size_t position() const = 0;
    virtual void close() = 0;
    virtual bool open() = 0;
    virtual ~MStream() = 0;
    bool isOpen();
private:
    bool m_isOpen;
};

class MIstream: public MStream {
public:
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;
    virtual size_t readBytes(char *buffer, size_t length) = 0;
    virtual size_t read(uint8_t* buf, size_t size) = 0;
};

class MOstream: public MStream {
public:
    virtual size_t write(uint8_t) = 0;
    virtual size_t write(const uint8_t *buf, size_t size) = 0;
    virtual void flush() = 0;
};

/********************************************************
 * Universal file
 ********************************************************/

class MFile {
public:
    MFile(nullptr_t null) : m_isNull(true) {};
    MFile(String path) : m_path(path) {};
    MFile(String path, String name);
    MFile(MFile* path, String name);

    const char* name() const;
    const char* path() const;
    const char* extension() const;
    bool operator!=(nullptr_t ptr);

    virtual bool isFile() = 0;
    virtual bool isDirectory() = 0;
    virtual MIstream* inputStream() = 0 ; // has to return OPENED stream
    virtual MOstream* outputStream() = 0 ; // has to return OPENED stream
    virtual time_t getLastWrite() = 0 ;
    virtual time_t getCreationTime() = 0 ;
    virtual void setTimeCallback(time_t (*cb)(void)) = 0 ;
    virtual bool rewindDirectory() = 0 ;
    virtual MFile* getNextFileInDir() = 0 ;
    virtual bool mkDir() = 0 ;
    virtual bool mkDirs() = 0 ;

protected:
    String m_path;

private:
    bool m_isNull;
};

/********************************************************
 * Filesystem instance
 * it knows how to create a MFile instance!
 ********************************************************/

class MFileSystem {
protected:
    char* m_prefix;

public:
    MFileSystem(char* prefix);
    bool services(String name);
    virtual MFile* create(String path) = 0;
};


/********************************************************
 * MFile factory
 ********************************************************/

class MFSOwner {
    static MFileSystem* availableFS[FS_COUNT];

public:
    static MFile* File(String name);
};

#endif