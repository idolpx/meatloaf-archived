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
    TCRTIStream(std::shared_ptr<MIStream> is) : CBMImageStream(is) 
    {
        // TCRT Offsets
        directory_header_offset = {0, 0, 24};  // Offset: 24x16
        directory_list_offset = {40, 3, 0x00};
        block_allocation_map = { {40, 1, 0x10, 1, 40, 6}, {40, 2, 0x10, 41, 80, 6} };
        sectorsPerTrack = { 40 };
    };

    //virtual uint16_t blocksFree() override;
	virtual uint8_t speedZone( uint8_t track) override { return 0; };

protected:
    struct TCRTHeader {
        char disk_name[24];
    };

    struct TCRTEntry {
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
    friend class TCRTFile;
};


/********************************************************
 * File implementations
 ********************************************************/

class TCRTFile: public D64File {
public:
    TCRTFile(std::string path, bool is_dir = true) : D64File(path, is_dir) {};

    MIStream* createIStream(std::shared_ptr<MIStream> containerIstream) override;
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
        return byExtension(".d81", fileName);
    }

    TCRTFileSystem(): MFileSystem("d81") {};
};


#endif /* MEATFILESYSTEM_MEDIA_TCRT */
