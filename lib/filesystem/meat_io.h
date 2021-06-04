#ifndef MEATFILE_DEFINES_H
#define MEATFILE_DEFINES_H

#include <memory>
#include <Arduino.h>
#include "FS.h"
#include "buffered_io.h"
#include "../make_unique.h"

#define FS_COUNT 1


/********************************************************
 * Universal file
 ********************************************************/

class MFile {
public:
    MFile(nullptr_t null) : m_isNull(true) {};
    MFile(String path);
    MFile(String path, String name);
    MFile(MFile* path, String name);

    const char* name() const;
    const char* path() const;
    const char* extension() const;
    bool operator!=(nullptr_t ptr);

    bool copyTo(MFile* dst) {
        std::unique_ptr<MIstream> istream(this->inputStream());
        std::unique_ptr<MOstream> ostream(dst->outputStream());

        auto br = std::make_unique<BufferedReader>(istream.get());
        auto bw = std::make_unique<BufferedWriter>(ostream.get());

        bool error = false;

        do {
            auto buffer = br->read();
            
            if(buffer->length() != 0) {
                Serial.printf("FSTEST: Bytes read into buffred reader: %d\n",buffer->length());
                int written = bw->write(buffer);
                Serial.printf("FSTEST: Bytes written into buffred writer: %d\n",written);
                error = buffer->length() != written;
            }
        } while (!br->eof());

        istream->close();
        ostream->close();

        return error;
    };

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
    virtual bool exists() = 0;
    virtual size_t size() = 0;
    virtual bool remove() = 0;
    virtual bool truncate(size_t size) = 0;
    virtual bool rename(const char* dest) = 0;
    virtual ~MFile() {};
protected:
    String m_path;
    bool m_isNull;
};

/********************************************************
 * Filesystem instance
 * it knows how to create a MFile instance!
 ********************************************************/

class MFileSystem {
public:
    MFileSystem(char* prefix);
    bool handles(String path);
    virtual bool mount() = 0;
    virtual bool umount() = 0;
    virtual MFile* getFile(String path) = 0;
    bool isMounted() {
        return m_isMounted;
    }

protected:
    char* protocol;
    bool m_isMounted;
};


/********************************************************
 * MFile factory
 ********************************************************/

class MFSOwner {
    static MFileSystem* availableFS[FS_COUNT];

public:
    static MFile* File(String name);
    static bool mount(String name);
    static bool umount(String name);
};

#endif