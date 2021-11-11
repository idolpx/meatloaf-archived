#include "tcrt.h"

/********************************************************
 * Streams
 ********************************************************/

bool TCRTIStream::seekEntry( size_t index )
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

size_t TCRTIStream::readFile(uint8_t* buf, size_t size) {
    size_t bytesRead = 0;

    // if ( sector_offset % block_size == 0 )
    // {
    //     // We are at the beginning of the block
    //     // Read track/sector link
    //     containerStream->read((uint8_t *)&next_track, 1);
    //     containerStream->read((uint8_t *)&next_sector, 1);
    //     sector_offset += 2;
    //     //Debug_printv("next_track[%d] next_sector[%d] sector_offset[%d]", next_track, next_sector, sector_offset);
    // }

    // bytesRead += containerStream->read(buf, size);
    // sector_offset += bytesRead;
    // m_bytesAvailable -= bytesRead;

    // if ( sector_offset % block_size == 0 )
    // {
    //     // We are at the end of the block
    //     // Follow track/sector link to move to next block
    //     seekSector( next_track, next_sector );
    //     //Debug_printv("track[%d] sector[%d] sector_offset[%d]", track, sector, sector_offset);
    // }


    return bytesRead;
}



bool TCRTIStream::seekPath(std::string path) {
    // Implement this to skip a queue of file streams to start of file by name
    // this will cause the next read to return bytes of 'path'
    seekCalled = true;

    entry_index = 0;

    // call D54Image method to obtain file bytes here, return true on success:
    // return D64Image.seekFile(containerIStream, path);
    mstr::toPETSCII(path);
    //if ( seekEntry(path) )
    {
        //auto entry = containerImage->entry;
        auto type = decodeType(entry.file_type).c_str();
        //auto blocks = (entry.blocks[0] << 8 | entry.blocks[1] >> 8);
        //auto blocks = (entry.blocks[0] * 256) + entry.blocks[1];
        //Debug_printv("filename [%.16s] type[%s] start_track[%d] start_sector[%d]", entry.filename, type, entry.start_track, entry.start_sector);
        //seekSector(entry.start_track, entry.start_sector);

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
            //seekSector( t, s );
        } while ( t > 0 );
        blocks--;
        m_length = (blocks * 254) + s - 2;
        m_bytesAvailable = m_length;
        
        // Set position to beginning of file
        //seekSector( entry.start_track, entry.start_sector );

        Debug_printv("File Size: blocks[%d] size[%d] available[%d]", (blocks + 1), m_length, m_bytesAvailable);
        
        return true;
    }
    //else
    {
        Debug_printv( "Not found! [%s]", path.c_str());
    }

    return false;
};

/********************************************************
 * File implementations
 ********************************************************/

MIStream* TCRTFile::createIStream(std::shared_ptr<MIStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new TCRTIStream(containerIstream);
}


bool TCRTFile::isDirectory() {
    //Debug_printv("pathInStream[%s]", pathInStream.c_str());
    if ( pathInStream == "" )
        return true;
    else
        return false;
};

bool TCRTFile::rewindDirectory() {
    dirIsOpen = true;
    Debug_printv("streamFile->url[%s]", streamFile->url.c_str());
    auto image = ImageBroker::obtain<TCRTIStream>(streamFile->url);
    if ( image == nullptr )
        Debug_printv("image pointer is null");

    image->resetEntryCounter();

    // Read Header
    image->seekHeader();

    // Set Media Info Fields
    media_header = mstr::format("%.24", image->header.disk_name);
    media_id = "tcrt";
    media_blocks_free = 0;
    media_block_size = image->block_size;
    media_image = name;

    return true;
}

MFile* TCRTFile::getNextFileInDir() {

    if(!dirIsOpen)
        rewindDirectory();

    // Get entry pointed to by containerStream
    auto image = ImageBroker::obtain<TCRTIStream>(streamFile->url);

    if ( image->seekNextImageEntry() )
    {
        std::string fileName = image->entry.filename;
        mstr::rtrimA0(fileName);
        mstr::replaceAll(fileName, "/", "\\");
        //Debug_printv( "entry[%s]", (streamFile->url + "/" + fileName).c_str() );
        auto d64_file = MFSOwner::File(streamFile->url + "/" + fileName);
        d64_file->extension = image->decodeType(image->entry.file_type);
        return d64_file;
    }
    else
    {
        //Debug_printv( "END OF DIRECTORY");
        dirIsOpen = false;
        return nullptr;
    }
}


size_t TCRTFile::size() {
    // Debug_printv("[%s]", streamFile->url.c_str());
    // use TCRT to get size of the file in image
    auto image = ImageBroker::obtain<TCRTIStream>(streamFile->url);
    // (_ui16 << 8 | _ui16 >> 8)
    //size_t blocks = (entry.blocks[0] << 8 | entry.blocks[1] >> 8);
    size_t blocks = (image->entry.end_address - image->entry.start_address) / image->block_size;
    //uint16_t blocks = entry.blocks[0] * 256 + entry.blocks[1];
    return blocks;
}
