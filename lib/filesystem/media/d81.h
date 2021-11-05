// .D81 - The D81 disk image format
// https://vice-emu.sourceforge.io/vice_17.html#SEC354
// https://ist.uwaterloo.ca/~schepers/formats/D81.TXT
//


#ifndef MEATFILESYSTEM_MEDIA_D81
#define MEATFILESYSTEM_MEDIA_D81

#include "meat_io.h"
#include "d64.h"

#include "string_utils.h"
#include <map>

#include "../../include/global_defines.h"

/********************************************************
 * Streams
 ********************************************************/

class D81IStream: public CBMImageStream {

public:
    D81IStream(std::shared_ptr<MIStream> is) : CBMImageStream(is) 
    {
        directory_header_offset = {40, 0};
        directory_list_offset = {40, 3};
        block_allocation_map = { {40, 1, 0x10, 1, 40, 6}, {40, 2, 0x00, 41, 80, 6} };
        sectorsPerTrack = { 40 };
    };

    virtual void seekHeader() override {
        Debug_printv("here");
        seekSector(directory_header_offset, 0x04);
    }

    //virtual uint16_t blocksFree() override;
	virtual uint8_t speedZone( uint8_t track) override { return 1; };

protected:
    // struct BAMEntry {
    //     uint8_t free_sectors;
    //     uint8_t sectors_00_07;
    //     uint8_t sectors_08_15;
    //     uint8_t sectors_16_23;
    //     uint8_t sectors_24_31;
    //     uint8_t sectors_32_39;
    // };

private:
    friend class D81File;
};


/********************************************************
 * File implementations
 ********************************************************/

class D81File: public D64File {
public:
    D81File(std::string path, bool is_dir = true) : D64File(path, is_dir) {};

    virtual MIStream* createIStream(std::shared_ptr<MIStream> containerIstream) override;
};



/********************************************************
 * FS
 ********************************************************/

class D81FileSystem: public MFileSystem
{
public:
    MFile* getFile(std::string path) override {
        return new D81File(path);
    }

    bool handles(std::string fileName) {
        //Serial.printf("handles w dnp %s %d\n", fileName.rfind(".dnp"), fileName.length()-4);
        return byExtension(".d81", fileName);
    }

    D81FileSystem(): MFileSystem("d81") {};
};


#endif /* MEATFILESYSTEM_MEDIA_D81 */
