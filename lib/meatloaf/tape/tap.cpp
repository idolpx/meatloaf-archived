#include "tap.h"

/********************************************************
 * Streams
 ********************************************************/

bool TAPIStream::seekEntry( std::string filename )
{
    uint8_t index = 1;
    mstr::rtrimA0(filename);
    mstr::replaceAll(filename, "\\", "/");

    // Read Directory Entries
    if ( filename.size() )
    {
        while ( seekEntry( index ) )
        {
            std::string entryFilename = entry.filename;
            mstr::rtrimA0(entryFilename);
            Debug_printv("filename[%s] entry.filename[%.16s]", filename.c_str(), entryFilename.c_str());

            // Read Entry From Stream
            if (filename == "*")
            {
                filename == entryFilename;
            }
            
            if ( mstr::compare(filename, entryFilename) )
            {
                // Move stream pointer to start track/sector
                return true;
            }
            index++;
        }
    }

    entry.filename[0] = '\0';

    return false;
}

bool TAPIStream::seekEntry( size_t index )
{
    // Calculate Sector offset & Entry offset
    index--;
    uint8_t entryOffset = 0x40 + (index * sizeof(entry));

    //Debug_printv("----------");
    //Debug_printv("index[%d] sectorOffset[%d] entryOffset[%d] entry_index[%d]", index, sectorOffset, entryOffset, entry_index);

    containerStream->seek(entryOffset);
    containerStream->read((uint8_t *)&entry, sizeof(entry));

    //Debug_printv("r[%d] file_type[%02X] file_name[%.16s]", r, entry.file_type, entry.filename);

    //if ( next_track == 0 && next_sector == 0xFF )
    entry_index = index + 1;    
    if ( entry.file_type == 0x00 )
        return false;
    else
        return true;
}


size_t TAPIStream::readFile(uint8_t* buf, size_t size) {
    size_t bytesRead = 0;

    bytesRead += containerStream->read(buf, size);
    m_bytesAvailable -= bytesRead;

    return bytesRead;
}

bool TAPIStream::seekPath(std::string path) {
    // Implement this to skip a queue of file streams to start of file by name
    // this will cause the next read to return bytes of 'path'
    seekCalled = true;

    entry_index = 0;

    // call image method to obtain file bytes here, return true on success:
    if ( seekEntry(path) )
    {
        //auto entry = containerImage->entry;
        auto type = decodeType(entry.file_type).c_str();
        size_t start_address = UINT16_FROM_LE_UINT16(entry.start_address);
        size_t end_address = UINT16_FROM_LE_UINT16(entry.end_address);
        size_t data_offset = UINT32_FROM_LE_UINT32(entry.data_offset);
        Debug_printv("filename [%.16s] type[%s] start_address[%d] end_address[%d] data_offset[%d]", entry.filename, type, start_address, end_address, data_offset);

        // Calculate file size
        m_length = ( end_address - start_address );
        m_bytesAvailable = m_length;

        // Set position to beginning of file
        containerStream->seek(entry.data_offset);

        Debug_printv("File Size: size[%d] available[%d]", m_length, m_bytesAvailable);
        
        return true;
    }
    else
    {
        Debug_printv( "Not found! [%s]", path.c_str());
    }

    return false;
};

/********************************************************
 * File implementations
 ********************************************************/

MStream* TAPFile::createIStream(std::shared_ptr<MStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new TAPIStream(containerIstream);
}


bool TAPFile::isDirectory() {
    //Debug_printv("pathInStream[%s]", pathInStream.c_str());
    if ( pathInStream == "" )
        return true;
    else
        return false;
};

bool TAPFile::rewindDirectory() {
    dirIsOpen = true;
    Debug_printv("streamFile->url[%s]", streamFile->url.c_str());
    auto image = ImageBroker::obtain<TAPIStream>(streamFile->url);
    if ( image == nullptr )
        Debug_printv("image pointer is null");

    image->resetEntryCounter();

    // Read Header
    image->seekHeader();

    // Set Media Info Fields
    media_header = mstr::format("%.24", image->header.disk_name);
    media_id = "tap";
    media_blocks_free = 0;
    media_block_size = image->block_size;
    media_image = name;
    mstr::toASCII(media_image);

    Debug_printv("media_header[%s] media_id[%s] media_blocks_free[%d] media_block_size[%d] media_image[%s]", media_header.c_str(), media_id.c_str(), media_blocks_free, media_block_size, media_image.c_str());

    return true;
}

MFile* TAPFile::getNextFileInDir() {

    if(!dirIsOpen)
        rewindDirectory();

    // Get entry pointed to by containerStream
    auto image = ImageBroker::obtain<TAPIStream>(streamFile->url);

    if ( image->seekNextImageEntry() )
    {
        std::string fileName = mstr::format("%.16s", image->entry.filename);
        mstr::replaceAll(fileName, "/", "\\");
        //Debug_printv( "entry[%s]", (streamFile->url + "/" + fileName).c_str() );
        auto file = MFSOwner::File(streamFile->url + "/" + fileName);
        file->extension = image->decodeType(image->entry.file_type);
        return file;
    }
    else
    {
        //Debug_printv( "END OF DIRECTORY");
        dirIsOpen = false;
        return nullptr;
    }
}


uint32_t TAPFile::size() {
    // Debug_printv("[%s]", streamFile->url.c_str());
    // use TAP to get size of the file in image
    auto image = ImageBroker::obtain<TAPIStream>(streamFile->url);

    size_t blocks = (UINT16_FROM_LE_UINT16(image->entry.end_address) - UINT16_FROM_LE_UINT16(image->entry.start_address)) / image->block_size;

    return blocks;
}
