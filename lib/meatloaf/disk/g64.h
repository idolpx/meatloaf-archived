// .G64 - The G64 GCR-encoded disk image format
// https://vice-emu.sourceforge.io/vice_17.html#SEC335
// https://ist.uwaterloo.ca/~schepers/formats/G64.TXT
// http://www.linusakesson.net/programming/gcr-decoding/index.php
//


#ifndef MEATLOAF_MEDIA_G64
#define MEATLOAF_MEDIA_G64

#include "meat_io.h"
#include "d64.h"

// Format codes:
// ID	Description
// 0	Unknown format
// 1	GCR Data
// 2	CBM DOS
// 3	CBM DOS Extended
// 4	MicroProse
// 5	RapidLok
// 6	Datasoft
// 7	Vorpal
// 8	V-MAX!
// 9	Teque
// 10	TDP
// 11	Big Five
// 12	OziSoft

// Format Extensions:
// ID	Description
// 0	Unknown protection
// 1	Datasoft with Weak bits
// 2	CBM DOS with Cyan loader, Weak bits
// 3	CBM DOS with Datasoft, Weak bits
// 4	RapidLok Key
// 5	Data Duplication
// 6	Melbourne House
// 7	Melbourne House, Weak bits
// 8	PirateBusters v1.0
// 9	PirateBusters v2.0, Track A
// 10	PirateBusters v2.0, Track B
// 11	PirateSlayer
// 12	CBM DOS, XEMAG

/********************************************************
 * Streams
 ********************************************************/

class G64IStream : public D64IStream {
    // override everything that requires overriding here

public:
    G64IStream(std::shared_ptr<MStream> is) : D64IStream(is) 
    {
        // G64 Offsets
        //directory_header_offset = {18, 0, 0x90};
        //directory_list_offset = {18, 1, 0x00};
        //block_allocation_map = { {18, 0, 0x04, 1, 35, 4}, {53, 0, 0x00, 36, 70, 3} };
        //sectorsPerTrack = { 17, 18, 19, 21 };
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

    MStream* createIStream(std::shared_ptr<MStream> containerIstream) override;
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


#endif /* MEATLOAF_MEDIA_G64 */
