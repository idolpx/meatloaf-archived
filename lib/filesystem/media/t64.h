// .T64 - The T64 tape image format
// https://vice-emu.sourceforge.io/vice_17.html#SEC331
// https://ist.uwaterloo.ca/~schepers/formats/T64.TXT
//


#ifndef MEATFILESYSTEM_MEDIA_T64
#define MEATFILESYSTEM_MEDIA_T64

#include "meat_io.h"
#include "d64.h"


/********************************************************
 * Streams
 ********************************************************/

class T64IStream : public CBMImageStream {
    // override everything that requires overriding here

public:
    T64IStream(std::shared_ptr<MIStream> is) : CBMImageStream(is) 
    {
        // T64 Offsets
        directory_header_offset = {1, 0, 0x28};
        directory_list_offset = {1, 0, 0x3F};
        sectorsPerTrack = { 40 };
    };

    virtual uint16_t blocksFree() override { return 0; };
	virtual uint8_t speedZone( uint8_t track) override { return 0; };

protected:
    struct T64Header {
        char disk_name[24];
    };

    struct T64Entry {
        uint8_t entry_type;
        uint8_t file_type;
        uint16_t start_address;
        uint16_t end_address;
        uint16_t free_1;
        uint32_t data_offset;
        uint32_t free_2;
        char filename[16];
    };

private:
    friend class T64File;
};


/********************************************************
 * File implementations
 ********************************************************/

class T64File: public D64File {
public:
    T64File(std::string path, bool is_dir = true) : D64File(path, is_dir) {};

    MIStream* createIStream(std::shared_ptr<MIStream> containerIstream) override;
};



/********************************************************
 * FS
 ********************************************************/

class T64FileSystem: public MFileSystem
{
public:
    MFile* getFile(std::string path) override {
        return new T64File(path);
    }

    bool handles(std::string fileName) {
        return byExtension(".t64", fileName);
    }

    T64FileSystem(): MFileSystem("t64") {};
};


#endif /* MEATFILESYSTEM_MEDIA_T64 */
