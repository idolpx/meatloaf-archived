
#include "archive_ml.h"

#include <string.h>
#include <archive.h>
#include <archive_entry.h>

#include "../meatloaf.h"

/* Returns pointer and size of next block of data from archive. */
// The read callback returns the number of bytes read, zero for end-of-file, or a negative failure code as above.
// It also returns a pointer to the block of data read.
// https://github.com/libarchive/libarchive/wiki/LibarchiveIO
//
// This callback is just a way to get bytes from srcStream into libarchive for processing
ssize_t cb_read(struct archive *a, void *userData, const void **buff)
{
    ArchiveMStreamData *streamData = (ArchiveMStreamData *)userData;
    // 1. we have to call srcStr.read(...)
    ssize_t bc = streamData->srcStream->read(streamData->srcBuffer, ArchiveMStream::buffSize);
    //std::string dump((char*)streamData->srcBuffer, bc);
    //Debug_printv("Libarch pulling data from src MStream, got bytes:%d", bc);
    //Debug_printv("Dumping bytes: %s", dump.c_str());
    // 2. set *buff to the bufer read in 1.
    *buff = streamData->srcBuffer;
    // 3. return read bytes count
    return bc;
}

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
// https://github.com/libarchive/libarchive/wiki/LibarchiveIO
int64_t cb_skip(struct archive *a, void *userData, int64_t request)
{
    Debug_printv("bytes[%d]", request);
    ArchiveMStreamData *streamData = (ArchiveMStreamData *)userData;

    if (streamData->srcStream->isOpen())
    {
        bool rc = streamData->srcStream->seek(request, SEEK_CUR);
        return (rc) ? request : ARCHIVE_WARN;
    }
    else
    {
        Debug_printv("ERROR! skip failed");
        return ARCHIVE_FATAL;
    }
}

int64_t cb_seek(struct archive *a, void *userData, int64_t offset, int whence)
{
    Debug_printv("offset[%d] whence[%d] (0=begin, 1=curr, 2=end)", offset, whence);
    ArchiveMStreamData *streamData = (ArchiveMStreamData *)userData;

    if (streamData->srcStream->isOpen())
    {
        bool rc = streamData->srcStream->seek(offset, whence);
        return (rc) ? offset : ARCHIVE_WARN;
    }
    else
    {
        Debug_printv("ERROR! seek failed");
        return ARCHIVE_FATAL;
    }
}

int cb_close(struct archive *a, void *userData)
{
    ArchiveMStreamData *src_str = (ArchiveMStreamData *)userData;
    
    Debug_printv("Libarch wants to close, but we do nothing here...");

    // do we want to close srcStream here???
    return (ARCHIVE_OK);
}

int cb_open(struct a *arch, void *userData)
{
    // maybe we can use open for something? Check if stream is open?
    return (ARCHIVE_OK);
}


/********************************************************
 * Streams implementations
 ********************************************************/

ArchiveMStream::ArchiveMStream(std::shared_ptr<MStream> srcStr)
{
    // it should be possible to to pass a password parameter here and somehow
    // call archive_passphrase_callback(password) from here, right?
    streamData.srcStream = srcStr;
    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    streamData.srcBuffer = new uint8_t[buffSize];

    open();
}

ArchiveMStream::~ArchiveMStream()
{
    close();
    if (streamData.srcBuffer != nullptr)
        delete[] streamData.srcBuffer;
    Debug_printv("Stream destructor OK!");
}

bool ArchiveMStream::open()
{
    if (!is_open)
    {
        // TODO enable seek only if the stream is random access
        archive_read_set_read_callback(a, cb_read);
        archive_read_set_seek_callback(a, cb_seek);
        //archive_read_set_skip_callback(a, cb_skip);
        archive_read_set_close_callback(a, cb_close);
        // archive_read_set_open_callback(mpa->arch, cb_open); - what does it do?
        archive_read_set_callback_data(a, &streamData);
        Debug_printv("== BEGIN Calling open1 on archive instance ==========================");
        int r =  archive_read_open1(a);
        Debug_printv("== END opening archive result=%d! (OK should be 0!) =======================================", r);

        //int r = archive_read_open2(a, &streamData, NULL, myRead, myskip, myclose);
        if (r == ARCHIVE_OK)
            is_open = true;
    }
    return is_open;
};

void ArchiveMStream::close()
{
    if (is_open)
    {
        archive_read_close(a);
        archive_read_free(a);
        is_open = false;
    }
    Debug_printv("Close called");
}

bool ArchiveMStream::isOpen()
{
    return is_open;
};

uint32_t ArchiveMStream::read(uint8_t *buf, uint32_t size)
{
    Debug_printv("calling read, buff size=[%d]", size);

    int r = archive_read_data(a, &buf, size);

    Debug_printv("archive returned [%d] unarchived bytes", r);
    if ( r >= 0 ) {
        _position += r;
        return r;
    }
    else if ( r == ARCHIVE_FATAL ) 
    {
        close();
        return 0;
    }
    else {
        return 0;
    }
}

uint32_t ArchiveMStream::write(const uint8_t *buf, uint32_t size)
{
    return -1;
}

// For files with a browsable random access directory structure
// d64, d74, d81, dnp, etc.
bool ArchiveMStream::seekPath(std::string path)
{
    Debug_printv("seekPath called for path: %s", path.c_str());

    if ( seekEntry( path ) )
    {
        Debug_printv("entry[%s]", archive_entry_pathname(entry));
        return true;
    }

    return false;
}


bool ArchiveMStream::seekEntry( std::string filename )
{
    Debug_printv( "filename[%s] size[%d]", filename.c_str(), filename.size());

    // Read Directory Entries
    if ( filename.size() > 0 )
    {
        bool found = false;
        bool wildcard =  ( mstr::contains(filename, "*") || mstr::contains(filename, "?") );
        while ( archive_read_next_header(a, &entry) == ARCHIVE_OK )
        {
            // Check filetype
            if ( archive_entry_filetype(entry) != AE_IFDIR )
            {
                std::string entryPath = ""; //archive_entry_sourcepath(entry);
                std::string entryFilename = basename(archive_entry_pathname(entry));

                Debug_printv("path[%s] filename[%s] entry.filename[%.16s]", entryPath.c_str(), filename.c_str(), entryFilename.c_str());

                // Read Entry From Stream
                if (filename == "*") // Match first entry
                {
                    filename = entryFilename;
                    found = true;
                }
                else if ( filename == entryFilename ) // Match exact
                {
                    found = true;
                }
                else if ( wildcard )
                {
                    if ( mstr::compare(filename, entryFilename) ) // X?XX?X* Wildcard match
                    {
                        // Set filename to this filename
                        Debug_printv( "Found! file[%s] -> entry[%s]", filename.c_str(), entryFilename.c_str() );
                        found = true;
                    }
                }

                if ( found )
                {
                    _size = archive_entry_size(entry);
                    
                    return true;
                }
            }
        }

        Debug_printv( "Not Found! file[%s]", filename.c_str() );
    }

    return false;
}

// // For files with no directory structure
// // tap, crt, tar
// std::string ArchiveMStream::seekNextEntry()
// {
//     struct archive_entry *entry;
//     if (archive_read_next_header(a, &entry) == ARCHIVE_OK)
//         return archive_entry_pathname(entry);
//     else
//         return "";
// };


bool ArchiveMStream::seek(uint32_t pos)
{
    return streamData.srcStream->seek(pos);
}

/********************************************************
 * Files implementations
 ********************************************************/

MStream *ArchiveMFile::getDecodedStream(std::shared_ptr<MStream> containerIstream)
{
    // TODO - we can get password from this URL and pass it as a parameter to this constructor
    Debug_printv("calling getDecodedStream for ArchiveMFile, we should return open stream");
    auto stream = new ArchiveMStream(containerIstream);
    return stream;
}

// archive file is always a directory
bool ArchiveMFile::isDirectory()
{
    //Debug_printv("pathInStream[%s]", pathInStream.c_str());
    if ( pathInStream == "" )
        return true;
    else
        return false;
};

bool ArchiveMFile::rewindDirectory()
{
    dirIsOpen = true;

    return prepareDirListing();
}

MFile *ArchiveMFile::getNextFileInDir()
{
    if(!dirIsOpen)
        rewindDirectory();

    struct archive_entry *entry;

    //Debug_printv("getNextFileInDir calling archive_read_next_header");
    if (archive_read_next_header(getArchive(), &entry) == ARCHIVE_OK)
    {
        std::string fileName = archive_entry_pathname(entry);
        auto file = MFSOwner::File(streamFile->url + "/" + fileName);
        file->_size = archive_entry_size(entry);
        file->_exists = true;
        return file;
    }
    else
    {
        //Debug_printv( "END OF DIRECTORY");
        dirStream->close();
        dirIsOpen = false;
        return nullptr;
    }
}

bool ArchiveMFile::prepareDirListing()
{
    if (dirStream.get() != nullptr)
    {
        dirStream->close();
    }

    Debug_printv("w prepare dir listing");

    dirStream = std::shared_ptr<MStream>(this->getSourceStream());

    if(dirStream->isOpen())
    {
        return true;
    }
    else
    {
        Debug_printv("opening Archive for dir nok");
        return false;
    }
}
