
#include "d64.h"

/********************************************************
 * File impls
 ********************************************************/

bool D64File::isDirectory() {
    //Debug_printv("pathInStream[%s]", pathInStream.c_str());
    if ( pathInStream == "" )
        return true;
    else
        return false;
};

bool D64File::rewindDirectory() {
    dirIsOpen = true;
    Debug_printv("streamFile->url[%s]", streamFile->url.c_str());
    auto image = ImageBroker::obtain(streamFile->url);
    if ( image == nullptr )
        Debug_printv("image pointer is null");

    image->resetEntryCounter();

    // Read Header
    image->seekHeader();

    // Set Media Info Fields
    media_header = mstr::format("%.16s", image->header.disk_name);
    mstr::A02Space(media_header);
    media_id = mstr::format("%.5s", image->header.id_dos);
    mstr::A02Space(media_id);
    media_blocks_free = image->blocksFree();
    media_block_size = image->block_size;
    media_image = name;

    return true;
}

MFile* D64File::getNextFileInDir() {

    if(!dirIsOpen)
        rewindDirectory();

    // Get entry pointed to by containerStream
    auto image = ImageBroker::obtain(streamFile->url);

    if ( image->seekNextImageEntry() )
    {
        std::string fileName = image->entry.filename;
        mstr::rtrimA0(fileName);
        mstr::replaceAll(fileName, "/", "\\");
        //Debug_printv( "entry[%s]", (streamFile->url + "/" + fileName).c_str() );
        auto d64_file = MFSOwner::File(streamFile->url + "/" + fileName);
        d64_file->extension = image->decodeEntry();
        return d64_file;
    }
    else
    {
        //Debug_printv( "END OF DIRECTORY");
        dirIsOpen = false;
        return nullptr;
    }
}

MIStream* D64File::createIStream(std::shared_ptr<MIStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new CBMImageStream(containerIstream);
}

time_t D64File::getLastWrite() {
    return getCreationTime();
}

time_t D64File::getCreationTime() {
    tm *entry_time = 0;
    auto entry = ImageBroker::obtain(streamFile->url)->entry;
    entry_time->tm_year = entry.year + 1900;
    entry_time->tm_mon = entry.month;
    entry_time->tm_mday = entry.day;
    entry_time->tm_hour = entry.hour;
    entry_time->tm_min = entry.minute;

    return mktime(entry_time);
}

bool D64File::exists() {
    // here I'd rather use D64 logic to see if such file name exists in the image!
    //Debug_printv("here");
    return true;
} 

size_t D64File::size() {
    // Debug_printv("[%s]", streamFile->url.c_str());
    // use D64 to get size of the file in image
    auto entry = ImageBroker::obtain(streamFile->url)->entry;
    // (_ui16 << 8 | _ui16 >> 8)
    //size_t blocks = (entry.blocks[0] << 8 | entry.blocks[1] >> 8);
    size_t blocks = UINT16_FROM_LE_UINT16(entry.blocks);
    //uint16_t blocks = entry.blocks[0] * 256 + entry.blocks[1];
    return blocks;
}

/********************************************************
 * Istream impls
 ********************************************************/


bool CBMImageStream::seekPath(std::string path) {
    // Implement this to skip a queue of file streams to start of file by name
    // this will cause the next read to return bytes of 'path'
    seekCalled = true;

    next_track = 0;
    next_sector = 0;
    sector_offset = 0;

    entry_index = 0;

    // call D54Image method to obtain file bytes here, return true on success:
    // return D64Image.seekFile(containerIStream, path);
    mstr::toPETSCII(path);
    if ( seekEntry(path) )
    {
        //auto entry = containerImage->entry;
        auto type = decodeEntry().c_str();
        //auto blocks = (entry.blocks[0] << 8 | entry.blocks[1] >> 8);
        //auto blocks = (entry.blocks[0] * 256) + entry.blocks[1];
        Debug_printv("filename [%.16s] type[%s] start_track[%d] start_sector[%d]", entry.filename, type, entry.start_track, entry.start_sector);
        seekSector(entry.start_track, entry.start_sector);

        // Calculate file size
        uint8_t t = 0;
        uint8_t s = 0;
        size_t blocks = 0; 
        do
        {
            //Debug_printv("t[%d] s[%d]", t, s);

            containerStream->read(&t, 1);
            containerStream->read(&s, 1);
            blocks++;
            seekSector( t, s );
        } while ( t > 0 );
        blocks--;
        m_length = (blocks * 254) + s - 2;
        m_bytesAvailable = m_length;
        
        // Set position to beginning of file
        seekSector( entry.start_track, entry.start_sector );

        Debug_printv("File Size: blocks[%d] size[%d] available[%d]", (blocks + 1), m_length, m_bytesAvailable);
        
        return true;
    }
    else
    {
        Debug_printv( "Not found! [%s]", path.c_str());
    }

    return false;
};

// std::string CBMImageStream::seekNextEntry() {
//     // Implement this to skip a queue of file streams to start of next file and return its name
//     // this will cause the next read to return bytes of "next" file in D64 image
//     // might not have sense in this case, as D64 is kinda random access, not a stream.
//     return "";
// };


bool CBMImageStream::open() {
    // return true if we were able to read the image and confirmed it is valid.
    // it's up to you in what state the stream will be after open. Could be either:
    // 1. EOF-like state (0 available) and the state will be cleared only after succesful seekNextEntry or seekPath
    // 2. non-EOF-like state, and ready to send bytes of first file, because you did immediate seekNextEntry here
    Debug_printv("here");
    return false;
};

void CBMImageStream::close() {

};

// bool CBMImageStream::seek(uint32_t pos) {
//     // seek only within current "active" ("seeked") file within the image (see above)
//     if(pos==m_position)
//         return true;

//     return false;
// };

size_t CBMImageStream::position() {
    return m_position; // return position within "seeked" file, not the D64 image!
};


int CBMImageStream::available() {
    // return bytes available in currently "seeked" file
    return m_bytesAvailable;
};

size_t CBMImageStream::size() {
    // size of the "seeked" file, not the image.
    return m_length;
};

size_t CBMImageStream::read(uint8_t* buf, size_t size) {
    size_t bytesRead = 0;

    if(seekCalled) {
        // if we have the stream set to a specific file already, either via seekNextEntry or seekPath, return bytes of the file here
        // or set the stream to EOF-like state, if whle file is completely read.
        bytesRead = readFile(buf, size);

    }
    else {
        // seekXXX not called - just pipe image bytes, so it can be i.e. copied verbatim
        bytesRead = containerStream->read(buf, size);
    }

    m_position += bytesRead;
    return bytesRead;
};

bool CBMImageStream::isOpen() {
    Debug_printv("here");
    return m_isOpen;
};


std::unordered_map<std::string, CBMImageStream*> ImageBroker::repo;