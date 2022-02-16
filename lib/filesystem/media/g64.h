// .G64 - The G64 GCR-encoded disk image format
// https://vice-emu.sourceforge.io/vice_17.html#SEC335
// https://ist.uwaterloo.ca/~schepers/formats/G64.TXT
//


#ifndef MEATFILESYSTEM_MEDIA_G64
#define MEATFILESYSTEM_MEDIA_G64

#include "meat_io.h"
#include "d64.h"


/********************************************************
 * Streams
 ********************************************************/

class G64IStream : public D64IStream {
    // override everything that requires overriding here

public:
    G64IStream(std::shared_ptr<MIStream> is) : D64IStream(is) 
    {
        // G64 Offsets
        //directory_header_offset = {18, 0, 0x90};
        //directory_list_offset = {18, 1, 0x00};
        block_allocation_map = { {18, 0, 0x04, 1, 35, 4}, {53, 0, 0x00, 36, 70, 3} };
        //sectorsPerTrack = { 17, 18, 19, 21 };
    };

    //virtual uint16_t blocksFree() override;
	virtual uint8_t speedZone( uint8_t track) override
	{
        if ( track < 35 )
		    return (track < 17) + (track < 24) + (track < 30);
        else
            return (track < 52) + (track < 59) + (track < 65);
	};

protected:

private:
    friend class G64File;
};


/********************************************************
 * File implementations
 ********************************************************/

class G64File: public D64File {
public:
    G64File(std::string path, bool is_dir = true) : D64File(path, is_dir) {};

    MIStream* createIStream(std::shared_ptr<MIStream> containerIstream) override;
};



/********************************************************
 * FS
 ********************************************************/

class G64FileSystem: public MFileSystem
{
public:
    MFile* getFile(std::string path) override {
        return new G64File(path);
    }

    bool handles(std::string fileName) {
        return byExtension(".g64", fileName);
    }

    G64FileSystem(): MFileSystem("g64") {};
};


#endif /* MEATFILESYSTEM_MEDIA_G64 */
