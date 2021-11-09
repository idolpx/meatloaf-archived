// .D64 - The D64 disk image format
// https://vice-emu.sourceforge.io/vice_17.html#SEC345
// https://ist.uwaterloo.ca/~schepers/formats/D64.TXT
// https://ist.uwaterloo.ca/~schepers/formats/GEOS.TXT
//

#ifndef MEATFILESYSTEM_MEDIA_D64
#define MEATFILESYSTEM_MEDIA_D64

#include "meat_io.h"
// #include "fs_littlefs.h"
// #include "MemoryInfo.h"
#include "string_utils.h"
#include <map>
#include <bitset>

#include "../../include/global_defines.h"

/********************************************************
 * Streams
 ********************************************************/

class CBMImageStream: public MIStream {

public:
    CBMImageStream(std::shared_ptr<MIStream> is) {
        containerStream = is;  
    }

    // MStream methods
    size_t position() override;
    void close() override;
    bool open() override;
    ~CBMImageStream() {
        //Debug_printv("close");
        close();
    }

    // MIStream methods
    bool isBrowsable() override { return false; };
    bool isRandomAccess() override { return true; };

    bool seek(int32_t pos, SeekMode mode) override { 
        Debug_printv("here");
        return true; 
    }; 
    bool seek(int32_t pos) override { 
        Debug_printv("here");
        return true; 
    };

    bool seekPath(std::string path) override;


    int available() override;
    size_t size() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen();

protected:
    struct Header {
        char disk_name[16];
        char unused[2];
        char id_dos[5];
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
        char filename[16];
        uint8_t rel_start_track;   // Or GOES info block start track
        uint8_t rel_start_sector;  // Or GEOS info block start sector
        uint8_t rel_record_length; // Or GEOS file structure (Sequential / VLIR file)
        uint8_t geos_type;         // $00 - Non-GEOS (normal C64 file)
        uint8_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t blocks[2];
    };


    // D64 Offsets
    std::vector<uint8_t> directory_header_offset = {18, 0, 0x90};
    std::vector<uint8_t> directory_list_offset = {18, 1, 0x00};
    std::vector<BAMInfo> block_allocation_map = { {18, 0, 0x04, 1, 35, 4} };
    std::vector<uint8_t> sectorsPerTrack = { 17, 18, 19, 21 };
    uint8_t sector_buffer[256] = { 0 };    
    uint16_t block_size = 256;

    bool seekCalled = false;
    std::shared_ptr<MIStream> containerStream;

    bool m_isOpen;
    int m_length;
    int m_bytesAvailable = 0;
    int m_position = 0;

    // D64Image methods
    CBMImageStream* decodedStream;

    size_t entry_index = 0;

    uint8_t dos_version;

    void seekHeader() {
        seekSector(directory_header_offset);
        containerStream->read((uint8_t*)&header, sizeof(header));
    }


    bool seekNextImageEntry() {
        return seekEntry(entry_index + 1);
    }

    void resetEntryCounter() {
        entry_index = 0;
    }

    std::string decodeEntry();
    bool seekSector( uint8_t track, uint8_t sector, size_t offset = 0 );
    bool seekSector( std::vector<uint8_t> trackSectorOffset = { 0 } );


    virtual uint16_t blocksFree();

	virtual uint8_t speedZone( uint8_t track)
	{
        Debug_printv("here");
		return (track < 30) + (track < 24) + (track < 17);
	};


    Header header;      // Directory header data
    Entry entry;        // Directory entry data

    enum open_modes { OPEN_READ, OPEN_WRITE, OPEN_APPEND, OPEN_MODIFY };
    std::string file_type_label[8] = { "del", "seq", "prg", "usr", "rel", "cbm", "dir", "???" };

    uint8_t track = 0;
    uint8_t sector = 0;
    uint16_t offset = 0;
    uint64_t blocks_free = 0;

    uint8_t next_track = 0;
    uint8_t next_sector = 0;
    uint8_t sector_offset = 0;

    uint8_t index = 0;  // Currently selected directory entry
    uint8_t length = 0; // Directory list entry count

    bool show_hidden = false;


private:

    size_t readFile(uint8_t* buf, size_t size);

    void sendListing();

    bool seekEntry( std::string filename );
    bool seekEntry( size_t index = 0 );


    std::string readBlock( uint8_t track, uint8_t sector );
    bool writeBlock( uint8_t track, uint8_t sector, std::string data );    
    bool allocateBlock( uint8_t track, uint8_t sector );
    bool deallocateBlock( uint8_t track, uint8_t sector );


    // uint8_t d64_get_type(uint16_t imgsize)
    // {
    //     switch (imgsize)
    //     {
    //         // D64
    //         case 174848:  // 35 tracks no errors
    //         case 175531:  // 35 w/ errors
    //         case 196608:  // 40 tracks no errors
    //         case 197376:  // 40 w/ errors
    //         case 205312:  // 42 tracks no errors
    //         case 206114:  // 42 w/ errors
    //             return D64_TYPE_D64;

    //         // D71
    //         case 349696:  // 70 tracks no errors
    //         case 351062:  // 70 w/ errors
    //             return D64_TYPE_D71;

    //         // D81
    //         case 819200:  // 80 tracks no errors
    //         case 822400:  // 80 w/ errors
    //             return D64_TYPE_D81;
    //     }

    //     return D64_TYPE_UNKNOWN;
    // }
    friend class D64File;
    friend class D71File;
    friend class D81File;
    friend class D8BFile;
    friend class DNPFile;
};

class D64IStream : public CBMImageStream {

};



/********************************************************
 * Utility implementations
 ********************************************************/
class ImageBroker {
    static std::unordered_map<std::string, CBMImageStream*> repo;
public:
    static CBMImageStream* obtain(std::string url) {
        // obviously you have to supply STREAMFILE.url to this function!
        if(repo.find(url)!=repo.end()) {
            return repo.at(url);
        }

        // create and add stream to broker if not found
        auto newFile = MFSOwner::File(url);
        CBMImageStream* newStream = (CBMImageStream*)newFile->inputStream();

        // Are we at the root of the pathInStream?
        if ( newFile->pathInStream == "")
        {
            Debug_printv("DIRECTORY [%s]", url.c_str());
        }
        else
        {
            Debug_printv("SINGLE FILE [%s]", url.c_str());
        } 

        repo.insert(std::make_pair(url, newStream));
        delete newFile;
        return newStream;
    }

    // static CBMImageStream* obtain(std::string url) {
    //     return obtain<CBMImageStream>(url);
    // }

    static void dispose(std::string url) {
        if(repo.find(url)!=repo.end()) {
            auto toDelete = repo.at(url);
            repo.erase(url);
            delete toDelete;
        }
    }
};

/********************************************************
 * File implementations
 ********************************************************/

class D64File: public MFile {
public:

    D64File(std::string path, bool is_dir = true): MFile(path) {
        isDir = is_dir;
    };
    
    ~D64File() {
        // don't close the stream here! It will be used by shared ptr D64Util to keep reading image params
    }

    virtual std::string petsciiName() {
        // It's already in PETSCII
        mstr::replaceAll(name, "\\", "/");
        return name;
    }

    bool isDirectory() override;
//    MIStream* inputStream() override { Debug_printv("here"); return nullptr; }; // has to return OPENED stream
//    MOStream* outputStream() override { Debug_printv("here"); return nullptr; }; // has to return OPENED stream

    virtual time_t getLastWrite() override;
    virtual time_t getCreationTime() override;
    virtual bool rewindDirectory() override;
    virtual MFile* getNextFileInDir() override;
    virtual bool mkDir() override { return false; };
    virtual bool exists() override;
    virtual size_t size() override;
    virtual bool remove() override { return false; };
    virtual bool rename(std::string dest) { return false; };

    virtual MIStream* createIStream(std::shared_ptr<MIStream> containerIstream);
    //void addHeader(const String& name, const String& value, bool first = false, bool replace = true);

    bool isDir = true;
    bool dirIsOpen = false;
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


#endif /* MEATFILESYSTEM_MEDIA_D64 */
