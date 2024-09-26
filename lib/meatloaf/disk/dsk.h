// .DSK - This is a byte for byte copy of a physical disk
//
// https://github.com/simonowen/samdisk/tree/main
// Apple - https://gswv.apple2.org.za/a2zine/faqs/Csa2FLUTILS.html#006
// Apple - http://fileformats.archiveteam.org/wiki/DSK_(Apple_II)
// Coleco Adam - https://retrocomputing.stackexchange.com/questions/15833/what-floppy-disk-format-and-layout-and-what-disk-image-format-are-used-for-the-c

#ifndef MEATLOAF_MEDIA_DSK
#define MEATLOAF_MEDIA_DSK

#include "../meatloaf.h"
#include "d64.h"


/********************************************************
 * Streams
 ********************************************************/

class DSKIStream : public D64MStream {
    // override everything that requires overriding here

public:
    DSKIStream(std::shared_ptr<MStream> is) : D64MStream(is) 
    {
        // DSK Partition Info
        std::vector<BlockAllocationMap> b = { 
            {
                40,     // track
                1,      // sector
                0x10,   // offset
                1,      // start_track
                40,     // end_track
                6       // byte_count
            }
        };

        Partition p = {
            35,    // track
            0,     // sector
            0x04,  // header_offset
            40,    // directory_track
            3,     // directory_sector
            0x00,  // directory_offset
            b      // block_allocation_map
        };
        partitions.clear();
        partitions.push_back(p);
        sectorsPerTrack = { 16 };
        dos_rom = "";
        dos_name = "";
        has_subdirs = false;
        error_info = true;

        uint32_t size = containerStream->size();
        switch (size + media_header_size) 
        {
            // Apple        // 35 Tracks, 16 sector/track, 256 bytes/sector
            case 146660:    
                break;

            // Coco         // 35 Tracks, 16 sector/track, 256 bytes/sector
            case 161280:
                break;

            // Apple        // 40 Tracks, 16 sector/track, 256 bytes/sector
            // Coleco Adam  // 40 Tracks, 8 sectors per track, 512 bytes per sectors
            case 163840:
                break;

            // Coco OS9
            case 184320:
                break;
        }
    };

    virtual uint8_t speedZone(uint8_t track) override { return 0; };

protected:

private:
    friend class DSKFile;
};


/********************************************************
 * File implementations
 ********************************************************/

class DSKFile: public D64MFile {
public:
    DSKFile(std::string path, bool is_dir = true) : D64MFile(path, is_dir) {};

    MStream* getDecodedStream(std::shared_ptr<MStream> containerIstream) override
    {
        Debug_printv("[%s]", url.c_str());

        return new DSKIStream(containerIstream);
    }

};



/********************************************************
 * FS
 ********************************************************/

class DSKFileSystem: public MFileSystem
{
public:
    MFile* getFile(std::string path) override {
        return new DSKFile(path);
    }

    bool handles(std::string fileName) override {
        return byExtension(
            {
                ".dsk",
                ".do",
                ".po",
                ".hdv"
            }, 
            fileName
        );
    }

    DSKFileSystem(): MFileSystem("dsk") {};
};


#endif /* MEATLOAF_MEDIA_DSK */
