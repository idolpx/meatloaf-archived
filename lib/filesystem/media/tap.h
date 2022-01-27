// .TAP - The raw tape image format
// https://vice-emu.sourceforge.io/vice_17.html#SEC330
// https://ist.uwaterloo.ca/~schepers/formats/TAP.TXT
// https://sourceforge.net/p/tapclean/gitcode/ci/master/tree/
// https://github.com/binaryfields/zinc64/blob/master/doc/Analyzing%20C64%20tape%20loaders.txt
// https://web.archive.org/web/20170117094643/http://tapes.c64.no/
// https://web.archive.org/web/20191021114418/http://www.subchristsoftware.com:80/finaltap.htm
//


#ifndef MEATFILESYSTEM_MEDIA_TAP
#define MEATFILESYSTEM_MEDIA_TAP

#include "meat_io.h"
#include "cbm_image.h"


/********************************************************
 * Streams
 ********************************************************/

class TAPIStream : public CBMImageStream {
    // override everything that requires overriding here

public:
    TAPIStream(std::shared_ptr<MIStream> is) : CBMImageStream(is) { };

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
        Debug_printv("here");
        containerStream->seek(0x28);
        containerStream->read((uint8_t*)&header, sizeof(header));
    }

    bool seekNextImageEntry() override {
        return seekEntry(entry_index + 1);
    }

    bool seekEntry( std::string filename ) override;
    bool seekEntry( size_t index ) override;

    size_t readFile(uint8_t* buf, size_t size) override;
    bool seekPath(std::string path) override;

    Header header;
    Entry entry;

private:
    friend class TAPFile;
};


/********************************************************
 * File implementations
 ********************************************************/

class TAPFile: public MFile {
public:

    TAPFile(std::string path, bool is_dir = true): MFile(path) {
        isDir = is_dir;
    };
    
    ~TAPFile() {
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

class TAPFileSystem: public MFileSystem
{
public:
    MFile* getFile(std::string path) override {
        return new TAPFile(path);
    }

    bool handles(std::string fileName) {
        return byExtension(".tap", fileName);
    }

    TAPFileSystem(): MFileSystem("tap") {};
};


#endif /* MEATFILESYSTEM_MEDIA_TAP */
