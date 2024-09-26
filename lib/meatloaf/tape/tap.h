// .TAP - The raw tape image format
//
// https://en.wikipedia.org/wiki/Commodore_Datasette
// https://vice-emu.sourceforge.io/vice_17.html#SEC330
// https://ist.uwaterloo.ca/~schepers/formats/TAP.TXT
// https://sourceforge.net/p/tapclean/gitcode/ci/master/tree/
// https://github.com/binaryfields/zinc64/blob/master/doc/Analyzing%20C64%20tape%20loaders.txt
// https://web.archive.org/web/20170117094643/http://tapes.c64.no/
// https://web.archive.org/web/20191021114418/http://www.subchristsoftware.com:80/finaltap.htm
//


#ifndef MEATLOAF_MEDIA_TAP
#define MEATLOAF_MEDIA_TAP

#include "../meatloaf.h"
#include "../meat_media.h"


/********************************************************
 * Streams
 ********************************************************/

class TAPMStream : public MMediaStream {
    // override everything that requires overriding here

public:
    TAPMStream(std::shared_ptr<MStream> is) : MMediaStream(is) { };

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
    bool seekEntry( uint16_t index ) override;

    uint16_t readFile(uint8_t* buf, uint16_t size) override;
    bool seekPath(std::string path) override;

    Header header;
    Entry entry;

private:
    friend class TAPMFile;
};


/********************************************************
 * File implementations
 ********************************************************/

class TAPMFile: public MFile {
public:

    TAPMFile(std::string path, bool is_dir = true): MFile(path) {
        isDir = is_dir;

        media_image = name;
        //mstr::toUTF8(media_image);
    };
    
    ~TAPMFile() {
        // don't close the stream here! It will be used by shared ptr D64Util to keep reading image params
    }

    MStream* getDecodedStream(std::shared_ptr<MStream> containerIstream) override;

    bool isDirectory() override;
    bool rewindDirectory() override;
    MFile* getNextFileInDir() override;
    bool mkDir() override { return false; };

    bool exists() override { return true; };
    bool remove() override { return false; };
    bool rename(std::string dest) override { return false; };
    time_t getLastWrite() override { return 0; };
    time_t getCreationTime() override { return 0; };
    uint32_t size() override;

    bool isDir = true;
    bool dirIsOpen = false;
};



/********************************************************
 * FS
 ********************************************************/

class TAPMFileSystem: public MFileSystem
{
public:
    MFile* getFile(std::string path) override {
        return new TAPMFile(path);
    }

    bool handles(std::string fileName) override {
        return byExtension(".tap", fileName);
    }

    TAPMFileSystem(): MFileSystem("tap") {};
};


#endif /* MEATLOAF_MEDIA_TAP */
