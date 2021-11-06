// .D71 - The D71 disk image format
// https://vice-emu.sourceforge.io/vice_17.html#SEC351
// https://ist.uwaterloo.ca/~schepers/formats/D71.TXT
//


#ifndef MEATFILESYSTEM_MEDIA_D71
#define MEATFILESYSTEM_MEDIA_D71

#include "meat_io.h"
#include "d64.h"

#include "string_utils.h"
#include <map>

#include "../../include/global_defines.h"

/********************************************************
 * Streams
 ********************************************************/

class D71IStream : public CBMImageStream {
    // override everything that requires overriding here

public:
    D71IStream(std::shared_ptr<MIStream> is) : CBMImageStream(is) 
    {
        // D71 Offsets
        directory_header_offset = {18, 0, 0x90};
        directory_list_offset = {18, 1, 0x00};
        block_allocation_map = { {18, 0, 0x04, 1, 35, 4}, {53, 0, 0x00, 36, 70, 3} };
        sectorsPerTrack = { 17, 18, 19, 21 };
    };

    //virtual uint16_t blocksFree() override;
	virtual uint8_t speedZone( uint8_t track)
	{
        if ( track < 36 )
		    return (track < 30) + (track < 24) + (track < 17);
        else
            return (track < 65) + (track < 59) + (track < 52);
	};

protected:

private:
    friend class D71File;
};


/********************************************************
 * File implementations
 ********************************************************/

class D71File: public D64File {
public:
    D71File(std::string path, bool is_dir = true) : D64File(path, is_dir) {};

    MIStream* createIStream(std::shared_ptr<MIStream> containerIstream) override;

    time_t getCreationTime() override;
    bool rewindDirectory() override;
    MFile* getNextFileInDir() override;
    bool exists() override;
    size_t size() override;
};



/********************************************************
 * FS
 ********************************************************/

class D71FileSystem: public MFileSystem
{
public:
    MFile* getFile(std::string path) override {
        return new D71File(path);
    }

    bool handles(std::string fileName) {
        //Serial.printf("handles w dnp %s %d\n", fileName.rfind(".dnp"), fileName.length()-4);
        return byExtension(".d71", fileName);
    }

    D71FileSystem(): MFileSystem("d71") {};
};


#endif /* MEATFILESYSTEM_MEDIA_D71 */
