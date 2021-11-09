// .D64 - The D64 disk image format
// https://vice-emu.sourceforge.io/vice_17.html#SEC345
// https://ist.uwaterloo.ca/~schepers/formats/D64.TXT
// https://ist.uwaterloo.ca/~schepers/formats/GEOS.TXT
//

#ifndef MEATFILESYSTEM_MEDIA_D64
#define MEATFILESYSTEM_MEDIA_D64

#include "meat_io.h"

#include <map>
#include <bitset>

#include "string_utils.h"
#include "cbm_image.h"


/********************************************************
 * Streams
 ********************************************************/

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
