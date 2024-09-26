#include "tap.h"

#include "meat_broker.h"
#include "endianness.h"

/********************************************************
 * Streams
 ********************************************************/

bool TAPMStream::seekEntry( std::string filename )
{
    uint8_t index = 1;
    uint8_t i = filename.find_first_of(0xA0);
    filename = filename.substr(0, i);
    //mstr::rtrimA0(filename);
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
                filename = entryFilename;
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

bool TAPMStream::seekEntry( uint16_t index )
{
    // Calculate Sector offset & Entry offset
    index--;
    uint16_t entryOffset = 0x40 + (index * sizeof(entry));

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


uint16_t TAPMStream::readFile(uint8_t* buf, uint16_t size) {
    uint16_t bytesRead = 0;

    bytesRead += containerStream->read(buf, size);
    _position += bytesRead;

    return bytesRead;
}

bool TAPMStream::seekPath(std::string path) {
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
        Debug_printv("filename [%.16s] type[%s] start_address[%zu] end_address[%zu] data_offset[%zu]", entry.filename, type, start_address, end_address, data_offset);

        // Calculate file size
        _size = ( end_address - start_address );

        // Set position to beginning of file
        containerStream->seek(entry.data_offset);

        Debug_printv("File Size: size[%d] available[%d]", _size, available());
        
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

MStream* TAPMFile::getDecodedStream(std::shared_ptr<MStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new TAPMStream(containerIstream);
}


bool TAPMFile::isDirectory() {
    //Debug_printv("pathInStream[%s]", pathInStream.c_str());
    if ( pathInStream == "" )
        return true;
    else
        return false;
};

bool TAPMFile::rewindDirectory() {
    dirIsOpen = true;
    Debug_printv("streamFile->url[%s]", streamFile->url.c_str());
    auto image = ImageBroker::obtain<TAPMStream>(streamFile->url);
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
    //mstr::toUTF8(media_image);

    Debug_printv("media_header[%s] media_id[%s] media_blocks_free[%d] media_block_size[%d] media_image[%s]", media_header.c_str(), media_id.c_str(), media_blocks_free, media_block_size, media_image.c_str());

    return true;
}

MFile* TAPMFile::getNextFileInDir() {

    if(!dirIsOpen)
        rewindDirectory();

    // Get entry pointed to by containerStream
    auto image = ImageBroker::obtain<TAPMStream>(streamFile->url);

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


uint32_t TAPMFile::size() {
    // Debug_printv("[%s]", streamFile->url.c_str());
    // use TAP to get size of the file in image
    auto image = ImageBroker::obtain<TAPMStream>(streamFile->url);

    size_t bytes = UINT16_FROM_LE_UINT16(image->entry.end_address) - UINT16_FROM_LE_UINT16(image->entry.start_address);

    return bytes;
}
