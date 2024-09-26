// .LBR - LiBRary containers
//
// https://ist.uwaterloo.ca/~schepers/formats/LBR.TXT
// https://github.com/talas/lbrtool
//


#ifndef MEATLOAF_MEDIA_LBR
#define MEATLOAF_MEDIA_LBR

#include "../meatloaf.h"
#include "../meat_media.h"


/********************************************************
 * Streams
 ********************************************************/

class LBRMStream : public MMediaStream {
    // override everything that requires overriding here

public:
    LBRMStream(std::shared_ptr<MStream> is) : MMediaStream(is) { 
        loadEntries();
    };

protected:
    struct Header {
        std::string disk_name;
        std::string id_dos;
    };

    struct Entry {
        std::string filename;
        uint16_t size;
        std::string type;
        uint32_t offset;
    };

    std::vector<Entry> entries;
    int8_t loadEntries();

    void seekHeader() override {
        header.disk_name = "LBR FILE";
        header.id_dos = " LBR ";
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
    friend class LBRMFile;
};


/********************************************************
 * File implementations
 ********************************************************/

class LBRMFile: public MFile {
public:

    LBRMFile(std::string path, bool is_dir = true): MFile(path) {
        isDir = is_dir;

        media_image = name;
        isPETSCII = true;
    };
    
    ~LBRMFile() {
        // don't close the stream here! It will be used by shared ptr D64Util to keep reading image params
    }

    MStream* getDecodedStream(std::shared_ptr<MStream> containerIstream) override
    {
        Debug_printv("[%s]", url.c_str());

        return new LBRMStream(containerIstream);
    }

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

class LBRMFileSystem: public MFileSystem
{
public:
    MFile* getFile(std::string path) override {
        return new LBRMFile(path);
    }

    bool handles(std::string fileName) override {
        return byExtension(".lbr", fileName);
    }

    LBRMFileSystem(): MFileSystem("lbr") {};
};


#endif /* MEATLOAF_MEDIA_LBR */
