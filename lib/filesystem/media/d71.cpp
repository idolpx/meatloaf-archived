#include "d71.h"

// D71 Utility Functions

MIStream* D71File::createIStream(std::shared_ptr<MIStream> containerIstream) {
    Debug_printv("[%s]", url.c_str());

    return new D71IStream(containerIstream);
}

bool D71File::rewindDirectory() {
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

MFile* D71File::getNextFileInDir() {

    if(!dirIsOpen)
        rewindDirectory();

    // Get entry pointed to by containerStream
    auto image = ImageBroker::obtain(streamFile->url);

    if ( image->seekNextImageEntry() )
    {
        std::string fileName = image->entry.filename;
        mstr::replaceAll(fileName, "/", "\\");
        Debug_printv( "entry[%s]", (streamFile->url + "/" + fileName).c_str() );
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

time_t D71File::getCreationTime() {
    tm *entry_time = 0;
    auto entry = ImageBroker::obtain(streamFile->url)->entry;
    entry_time->tm_year = entry.year + 1900;
    entry_time->tm_mon = entry.month;
    entry_time->tm_mday = entry.day;
    entry_time->tm_hour = entry.hour;
    entry_time->tm_min = entry.minute;

    return mktime(entry_time);
}

bool D71File::exists() {
    // here I'd rather use D64 logic to see if such file name exists in the image!
    Debug_printv("here");
    return true;
} 

size_t D71File::size() {
    // Debug_printv("[%s]", streamFile->url.c_str());
    // use D64 to get size of the file in image
    auto entry = ImageBroker::obtain(streamFile->url)->entry;
    // (_ui16 << 8 | _ui16 >> 8)
    //uint16_t blocks = (entry.blocks[0] << 8 | entry.blocks[1] >> 8);
    uint16_t blocks = entry.blocks[0] * 256 + entry.blocks[1];
    return blocks;
}