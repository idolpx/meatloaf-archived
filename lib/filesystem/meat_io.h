#ifndef MEATFILE_DEFINES_H
#define MEATFILE_DEFINES_H

#include <memory>
#include <Arduino.h>
#include <string>
#include <vector>
#include "FS.h"
#include "buffered_io.h"
#include "meat_stream.h"
#include "../../include/make_unique.h"
#include "EdUrlParser.h"

/********************************************************
 * Universal file
 ********************************************************/

class MFile : public EdUrlParser {
public:
    MFile(nullptr_t null) : m_isNull(true) {};
    MFile(std::string path);
    MFile(std::string path, std::string name);
    MFile(MFile* path, std::string name);
    static std::vector<std::string> chopPath(std::string path);


    //std::string name();
    //std::string path();
    //std::string extension();
    std::string media_root;
    std::string media_header;
    std::string media_id;
    std::string media_image;
    uint16_t media_blocks_free;
    uint16_t media_block_size;
    

    bool operator!=(nullptr_t ptr);

    bool copyTo(MFile* dst) {
        std::unique_ptr<MIstream> istream(this->inputStream());
        std::unique_ptr<MOstream> ostream(dst->outputStream());

        return istream->pipeTo(ostream.get());
    };

    std::vector<std::string> chop();

    virtual bool isDirectory() = 0;
    virtual MIstream* inputStream();
    virtual MOstream* outputStream() = 0 ; // has to return OPENED stream
    virtual time_t getLastWrite() = 0 ;
    virtual time_t getCreationTime() = 0 ;
    virtual bool rewindDirectory() = 0 ;
    virtual MFile* getNextFileInDir() = 0 ;
    virtual bool mkDir() = 0 ;
    virtual bool exists() = 0;
    virtual size_t size() = 0;
    virtual bool remove() = 0;
    virtual bool rename(const char* dest) = 0;
    virtual MFile* getNextEntry() {};
    virtual bool isBrowsable() { return false; };
    virtual bool isRandomAccess() { return false; };

    virtual ~MFile() {};

    std::string streamPath;
    std::string pathInStream;
protected:
    virtual MIstream* createIStream(MIstream* src) = 0;
    std::string m_path;
    bool m_isNull;
    void fillPaths(std::vector<std::string>::iterator* matchedElement, std::vector<std::string>::iterator* fromStart, std::vector<std::string>::iterator* last);
friend class MFSOwner;
};

/********************************************************
 * Filesystem instance
 * it knows how to create a MFile instance!
 ********************************************************/

class MFileSystem {
public:
    MFileSystem(char* symbol);
    virtual ~MFileSystem() = 0;
    virtual bool mount() { return true; };
    virtual bool umount() { return true; };
    virtual bool handles(std::string path) = 0;
    virtual MFile* getFile(std::string path) = 0;
    bool isMounted() {
        return m_isMounted;
    }

    static bool byExtension(char* ext, std::string fileName) {
        return fileName.rfind(ext) != -1 &&  toupper(fileName.rfind(ext)) == toupper(fileName.length()-4);
    }

protected:
    char* symbol;
    bool m_isMounted;

    friend class MFSOwner;
};


/********************************************************
 * MFile factory
 ********************************************************/

class MFSOwner {
    static std::vector<MFileSystem*> availableFS;

public:
    static MFile* File(std::string name);
    static bool mount(std::string name);
    static bool umount(std::string name);
};


// class Browsable {
// public:
//     virtual MFile* getNextEntry() = 0;
// };


#endif