#ifndef MEATFILE_DEFINES_H
#define MEATFILE_DEFINES_H

#include <memory>
#include <Arduino.h>
#include "FS.h"

#define FS_COUNT 1

class MStream 
{
public:
    bool seek(uint32_t pos, SeekMode mode);
    bool seek(uint32_t pos);
    size_t position() const;
    void close();
};

class MIstream: public MStream {
public:
    int available();
    int read();
    int peek();
    size_t readBytes(char *buffer, size_t length);
    size_t read(uint8_t* buf, size_t size);
};

class MOstream: public MStream {
public:
    size_t write(uint8_t);
    size_t write(const uint8_t *buf, size_t size);
    void flush();
};

class MFile {
public:
    MFile(nullptr_t null) : m_isNull(true) {};
    MFile(String path) : m_path(path) {};
    MFile(String name, String subDir);
    MFile(MFile name, String subDir);
    const char* name() const;
    const char* path() const;
    const char* extension() const;
    bool isFile() const;
    bool isDirectory() const;
    MIstream inputStream();
    MOstream outputStream();
    time_t getLastWrite();
    time_t getCreationTime();
    void setTimeCallback(time_t (*cb)(void));
    bool rewindDirectory();
    MFile openNextFile();
    bool mkDir();
    bool mkDirs();
    bool operator!=(nullptr_t ptr);

protected:
    String m_path;

private:
    bool m_isNull;
};

class MFileSystem {
protected:
    char* m_prefix;

public:
    MFileSystem(char* prefix);
    bool services(String name);
    virtual MFile create(String path) = 0;
};


class MFSOwner {
    static MFileSystem* availableFS[FS_COUNT];

public:
    static MFile File(String name);
};

#endif