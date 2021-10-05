// .D64 - The D64 disk image format
// https://vice-emu.sourceforge.io/vice_17.html#SEC345
// https://ist.uwaterloo.ca/~schepers/formats/D64.TXT
// https://ist.uwaterloo.ca/~schepers/formats/GEOS.TXT
//

#ifndef MEATFILE_DEFINES_D64_H
#define MEATFILE_DEFINES_D64_H

#include "meat_io.h"

#include "../../include/global_defines.h"


/********************************************************
 * File implementations
 ********************************************************/

class D64File: public MFile {

protected:

    uint8_t directory_header_offset[2] = {18, 0};
    uint8_t directory_list_offset[2] = {18, 1};
    uint8_t block_allocation_map[6] = {18, 0, 0x04, 1, 35, 4};
    uint16_t block_size = 256;

    MIStream* containerStream;

public:

    struct Header {
        uint8_t dos_version;
        std::string disk_name[16];
        std::string id_dos[5];
    };

    struct BAMInfo {
        uint8_t track;
        uint8_t sector;
        uint8_t offset;
        uint8_t start_track;
        uint8_t end_track;
        uint8_t byte_count;
    };

    struct Entry {
        uint8_t next_track;
        uint8_t next_sector;
        uint8_t file_type;
        uint8_t start_track;
        uint8_t start_sector;
        std::string filename;
        uint8_t rel_start_track;   // Or GOES info block start track
        uint8_t rel_start_sector;  // Or GEOS info block start sector
        uint8_t rel_record_length; // Or GEOS file structure (Sequential / VLIR file)
        uint8_t geos_type;         // $00 - Non-GEOS (normal C64 file)
        uint8_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint16_t blocks;
    };

    std::string file_type_label[7] = { "DEL", "SEQ", "PRG", "USR", "REL", "CBM", "DIR" };

    D64File(std::string path): MFile(path) {

        std::unique_ptr<MFile> containerFile(MFSOwner::File(streamPath)); // get the base file that knows how to handle this kind of container
        containerStream = containerFile->inputStream();

        // Read Header
        //Header diskHeader;
        //containerStream->read(diskHeader, sizeof(diskHeader));
        // Count Directory Entries
        // Calculate Blocks Free

    };
    
    ~D64File() {

        containerStream->close();
    }

    uint8_t track;
    uint8_t sector;
    uint16_t offset;
    uint64_t blocks_free;

    uint8_t index = 0;  // Currently selected directory entry
    uint8_t length = 0; // Directory list entry count
    Entry entry;        // Directory entry data

    bool show_hidden = false;

    void sendListing();
    void sendFile( std::string filename = "" );

    bool seekSector( uint8_t track, uint8_t sector, uint16_t offset = 0 );
    bool seekSector( uint8_t trackSector[], uint16_t offset = 0 );
    Entry seekFile( std::string filename );    

    std::string readBlock( uint8_t track, uint8_t sector );
    bool writeBlock( uint8_t track, uint8_t sector, std::string data );    
    bool allocateBlock( uint8_t track, uint8_t sector );
    bool deallocateBlock( uint8_t track, uint8_t sector );

    bool isDirectory() override;
//    MIStream* inputStream() override { return nullptr; }; // has to return OPENED stream
//    MOStream* outputStream() override { return nullptr; }; // has to return OPENED stream
    time_t getLastWrite() override;
    time_t getCreationTime() override;
    bool rewindDirectory() override { return false; };
    MFile* getNextFileInDir() override { return nullptr; };
    bool mkDir() override { return false; };
    bool exists() override;
    size_t size() override;
    bool remove() override { return false; };
    bool rename(std::string dest) { return false; };
    MIStream* createIStream(MIStream* src);
    //void addHeader(const String& name, const String& value, bool first = false, bool replace = true);

protected:
    void fillPaths(std::vector<std::string>::iterator* matchedElement, std::vector<std::string>::iterator* fromStart, std::vector<std::string>::iterator* last);
};


/********************************************************
 * Streams
 ********************************************************/

class D64IStream: public MIStream {

public:
    D64IStream(std::string path) {
        url = path;
    }
    // MStream methods
    size_t position() override;
    void close() override;
    bool open() override;
    ~D64IStream() {
        close();
    }

    // MIStream methods
    bool isBrowsable() override { return true; };
    bool isRandomAccess() override { return true; };

    bool seek(uint32_t pos, SeekMode mode) override { return true; }; 
    bool seek(uint32_t pos) override { return true; };
    int available() override;
    size_t size() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen();

protected:
    std::string url;
    std::unique_ptr<MFile> m_mfile;

    bool m_isOpen;
    int m_length;
    int m_bytesAvailable = 0;
    int m_position = 0;
};

/********************************************************
 * FS
 ********************************************************/

class D64FileSystem: public MFileSystem
{
public:
    MFile* getFile(std::string path) override {
        return new D64File(path);
    }

    bool handles(std::string fileName) {
        //Serial.printf("handles w dnp %s %d\n", fileName.rfind(".dnp"), fileName.length()-4);
        return byExtension(".d64", fileName);
    }

    D64FileSystem(): MFileSystem("d64") {};
};


#endif