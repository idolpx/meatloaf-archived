// .TCRT - Tapecart File System
// https://github.com/ikorb/tapecart
// https://github.com/ikorb/tapecart/blob/master/doc/TCRT%20Format.md
//


#ifndef MEATFILESYSTEM_MEDIA_TCRT
#define MEATFILESYSTEM_MEDIA_TCRT

#include "meat_io.h"
#include "d64.h"


/********************************************************
 * Streams
 ********************************************************/

class TCRTIStream : public CBMImageStream {
    // override everything that requires overriding here

public:
    TCRTIStream(std::shared_ptr<MIStream> is) : CBMImageStream(is) {};

protected:
    struct Header {
        char disk_name[24];
    };

    struct Entry {
        uint8_t entry_type;
        uint8_t file_type;
        uint16_t start_address;
        uint16_t end_address;
        uint16_t free_1;
        uint32_t data_offset;
        uint32_t free_2;
        char filename[16];
    };

    void seekHeader() override {
        containerStream->seek(0x18);
        containerStream->read((uint8_t*)&header, sizeof(header));
    }

    bool seekNextImageEntry() override {
        return containerStream->seek(0x40 + (entry_index + 1 * 32));
    }

    bool seekEntry( size_t index ) override;
    size_t readFile(uint8_t* buf, size_t size) override;
    bool seekPath(std::string path) override;

    Header header;
    Entry entry;

private:
    friend class TCRTFile;
};


/********************************************************
 * File implementations
 ********************************************************/

class TCRTFile: public MFile {
public:

    TCRTFile(std::string path, bool is_dir = true): MFile(path) {
        isDir = is_dir;
    };
    
    ~TCRTFile() {
        // don't close the stream here! It will be used by shared ptr D64Util to keep reading image params
    }

    MIStream* createIStream(std::shared_ptr<MIStream> containerIstream) override;

    std::string petsciiName() override {
        // It's already in PETSCII
        mstr::replaceAll(name, "\\", "/");
        return name;
    }

    bool isDirectory() override;
    bool rewindDirectory() override;
    MFile* getNextFileInDir() override;
    bool mkDir() override { return false; };

    bool exists() override { return true; };
    bool remove() override { return false; };
    bool rename(std::string dest) { return false; };
    time_t getLastWrite() override { return 0; };
    time_t getCreationTime() override { return 0; };
    size_t size() override;

    bool isDir = true;
    bool dirIsOpen = false;
};



/********************************************************
 * FS
 ********************************************************/

class TCRTFileSystem: public MFileSystem
{
public:
    MFile* getFile(std::string path) override {
        return new TCRTFile(path);
    }

    bool handles(std::string fileName) {
        return byExtension(".tcrt", fileName);
    }

    TCRTFileSystem(): MFileSystem("tcrt") {};
};


#endif /* MEATFILESYSTEM_MEDIA_TCRT */
