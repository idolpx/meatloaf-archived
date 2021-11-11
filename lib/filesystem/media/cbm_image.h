


#ifndef MEATFILESYSTEM_MEDIA_CBM_IMAGE
#define MEATFILESYSTEM_MEDIA_CBM_IMAGE

#include "meat_io.h"

#include <map>
#include <bitset>

#include "string_utils.h"



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

    bool seekPath(std::string path) override { return false; };
    std::string seekNextEntry() override { return ""; };

    int available() override;
    size_t size() override;
    size_t read(uint8_t* buf, size_t size) override;
    bool isOpen();

protected:

    // struct Header {
    //     char disk_name[16];
    //     char id_dos[5];
    // };

    // struct Entry {
    //     uint8_t next_track;
    //     uint8_t next_sector;
    //     uint8_t file_type;
    //     uint8_t start_track;
    //     uint8_t start_sector;
    //     char filename[16];
    //     uint8_t rel_start_track;   // Or GOES info block start track
    //     uint8_t rel_start_sector;  // Or GEOS info block start sector
    //     uint8_t rel_record_length; // Or GEOS file structure (Sequential / VLIR file)
    //     uint8_t geos_type;         // $00 - Non-GEOS (normal C64 file)
    //     uint8_t year;
    //     uint8_t month;
    //     uint8_t day;
    //     uint8_t hour;
    //     uint8_t minute;
    //     uint16_t blocks;
    // };

    // Header header;
    // Entry entry;

    bool seekCalled = false;
    std::shared_ptr<MIStream> containerStream;

    bool m_isOpen;
    int m_length;
    int m_bytesAvailable = 0;
    int m_position = 0;

    // D64Image methods
    CBMImageStream* decodedStream;

    bool show_hidden = false;

    size_t block_size = 256;
    size_t entry_index = 0;
    uint8_t index = 0;  // Currently selected directory entry
    uint8_t length = 0; // Directory list entry count

    enum open_modes { OPEN_READ, OPEN_WRITE, OPEN_APPEND, OPEN_MODIFY };
    std::string file_type_label[8] = { "del", "seq", "prg", "usr", "rel", "cbm", "dir", "???" };

    virtual void seekHeader() = 0;
    virtual bool seekNextImageEntry() = 0;
    void resetEntryCounter() {
        entry_index = 0;
    }

    // Disks
    virtual uint16_t blocksFree() { return 0; };
	virtual uint8_t speedZone( uint8_t track) { return 0; };

    virtual bool seekEntry( size_t index ) { return false; };
    virtual size_t readFile(uint8_t* buf, size_t size) = 0;
    std::string decodeType(uint8_t file_type, bool show_hidden = false);    

private:


    // File

    // Disk
    friend class D64File;
    friend class D71File;
    friend class D81File;
    friend class D8BFile;
    friend class DNPFile; 

    // Tape
    friend class T64File;
    friend class TCRTFile;

    // Cartridge

};



/********************************************************
 * Utility implementations
 ********************************************************/
class ImageBroker {
    static std::unordered_map<std::string, CBMImageStream*> repo;
public:
    template<class T> static T* obtain(std::string url) {
        // obviously you have to supply STREAMFILE.url to this function!
        if(repo.find(url)!=repo.end()) {
            return (T*)repo.at(url);
        }

        // create and add stream to broker if not found
        auto newFile = MFSOwner::File(url);
        T* newStream = (T*)newFile->inputStream();

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

    static CBMImageStream* obtain(std::string url) {
        return obtain<CBMImageStream>(url);
    }

    static void dispose(std::string url) {
        if(repo.find(url)!=repo.end()) {
            auto toDelete = repo.at(url);
            repo.erase(url);
            delete toDelete;
        }
    }
};

#endif