// .7Z, .ARC, .ARK, .BZ2, .GZ, .LHA, .LZH, .LZX, .RAR, .TAR, .TGZ, .XAR, .ZIP - libArchive for Meatloaf!
//
// https://stackoverflow.com/questions/22543179/how-to-use-libarchive-properly
// https://libarchive.org/

#ifndef MEATLOAF_ARCHIVE
#define MEATLOAF_ARCHIVE

#include <archive.h>
#include <archive_entry.h>

#include "../meatloaf.h"

#include "../../../include/debug.h"

// TODO: check how we can use archive_seek_callback, archive_passphrase_callback etc. to our benefit!

/* Returns pointer and size of next block of data from archive. */
// The read callback returns the number of bytes read, zero for end-of-file, or a negative failure code as above.
// It also returns a pointer to the block of data read.
ssize_t cb_read(struct archive *a, void *__src_stream, const void **buff);

/*
It must return the number of bytes actually skipped, or a negative failure code if skipping cannot be done.
It can skip fewer bytes than requested but must never skip more.
Only positive/forward skips will ever be requested.
If skipping is not provided or fails, libarchive will call the read() function and simply ignore any data that it does not need.

* Skips at most request bytes from archive and returns the skipped amount.
* This may skip fewer bytes than requested; it may even skip zero bytes.
* If you do skip fewer bytes than requested, libarchive will invoke your
* read callback and discard data as necessary to make up the full skip.
*/
int64_t cb_skip(struct archive *a, void *__src_stream, int64_t request);

int64_t cb_seek(struct archive *a, void *userData, int64_t offset, int whence);
int cb_close(struct archive *a, void *__src_stream);


/********************************************************
 * Streams implementations
 ********************************************************/

class ArchiveMStreamData {
public:
    uint8_t *srcBuffer = nullptr;
    std::shared_ptr<MStream> srcStream = nullptr; // a stream that is able to serve bytes of this archive
};

class ArchiveMStream : public MStream
{
    bool is_open = false;

public:
    static const size_t buffSize = 4096;
    ArchiveMStreamData streamData;
    struct archive *a;
    struct archive_entry *entry;

    ArchiveMStream(std::shared_ptr<MStream> srcStr);
    ~ArchiveMStream();

    bool open() override;
    void close() override;
    bool isOpen() override;

    uint32_t read(uint8_t *buf, uint32_t size) override;
    uint32_t write(const uint8_t *buf, uint32_t size) override;

    // For files with a browsable random access directory structure
    // d64, d74, d81, dnp, etc.
    bool seekPath(std::string path) override;

    // For files with no directory structure
    // tap, crt, tar
    //std::string seekNextEntry() override;

    virtual bool seek(uint32_t pos) override;

    bool isRandomAccess() override { return true; };
    bool seekEntry( std::string filename );

protected:


private:

};

/********************************************************
 * Files implementations
 ********************************************************/

class ArchiveMFile : public MFile
{
    struct archive *getArchive() {
        ArchiveMStream* as = (ArchiveMStream*)dirStream.get();
        return as->a;
    }
    
    std::shared_ptr<MStream> dirStream = nullptr; // a stream that is able to serve bytes of this archive

public:
    ArchiveMFile(std::string path) : MFile(path)
    {
        // media_header = name;
        media_archive = name;
    };

    ~ArchiveMFile()
    {
        if (dirStream.get() != nullptr)
        {
            dirStream->close();
        }
    }

    std::string basepath = "";

    //MStream* getSourceStream(std::ios_base::openmode mode=std::ios_base::in) override; // has to return OPENED stream
    MStream* getDecodedStream(std::shared_ptr<MStream> src) override;

    // archive file is always a directory
    bool isDirectory() override;
    bool rewindDirectory() override;
    MFile *getNextFileInDir() override;

    bool mkDir() override { return false; };
    bool remove() override { return true; }
    bool rename(std::string dest) override { return true; }

    time_t getLastWrite() override { return 0; }
    time_t getCreationTime() override { return 0; }

private:
    bool prepareDirListing();

    bool isDir = true;
    bool dirIsOpen = false;
};

/********************************************************
 * FS implementations
 ********************************************************/

class ArchiveMFileSystem : public MFileSystem
{
    MFile *getFile(std::string path)
    {
        return new ArchiveMFile(path);
    };

public:
    ArchiveMFileSystem() : MFileSystem("arch") {}

    bool handles(std::string fileName)
    {
        return byExtension(
            {
                ".7z",
                ".arc",
                ".ark",
                ".bz2",
                ".gz",
                ".lha",
                ".lzh",
                ".lzx",
                ".rar",
                ".tar",
                ".tgz",
                ".xar",
                ".zip"
             },
            fileName
        );
    }

private:
};

#endif // MEATLOAF_ARCHIVE