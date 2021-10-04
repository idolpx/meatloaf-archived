#include "d64.h"

// D64 Utility Functions

bool D64File::seekSector( uint8_t track, uint8_t sector, uint8_t offset)
{
    track--;
    uint8_t sector_count = 0;

    if ( track < 17 )
    {
        sector_count += (track * 21) + sector;
    }
    else if ( track < 24 )
    {
        sector_count = 357;
        sector_count += ((track - 17) * 19) + sector;
    }
    else if ( track < 30 )
    {
        sector_count = 490;
        sector_count += ((track - 24) * 18) + sector;
    }
    else if ( track > 29 )
    {
        sector_count = 598;
        sector_count += ((track - 30) * 17) + sector;
    }
    
    // fseek(this->fp, (sector_count * 256));
    // return ftell($this->fp);
    return true;
}

bool D64File::seekSector( uint8_t trackSector[], uint8_t offset = 0 )
{
    return seekSector(trackSector[0], trackSector[1], offset);
}

std::string D64File::readBlock(uint8_t track, uint8_t sector)
{

}

bool D64File::writeBlock(uint8_t track, uint8_t sector, std::string data)
{

}

bool D64File::allocateBlock( uint8_t track, uint8_t sector)
{

}

bool D64File::deallocateBlock( uint8_t track, uint8_t sector)
{

}


void D64File::sendListing() 
{
    uint8_t index = 0;
    Entry entry;

    sendHeader();
    
    // Read Directory Entries
    seekSector( directory_list_offset );
    do
    {		
        // Read Entry From Stream
        
        //echo ord($file_type); exit();
        bool hide = false;
        uint8_t file_type = entry.file_type & 0b00000111;
        std::string type = file_type_label[ file_type ];
        if ( file_type == 0 )
            hide = true;

        switch ( file_type & 0b11000000 )
        {
            case 0xC0:			// Bit 6: Locked flag (Set produces "<" locked files)
                type += "<";
                hide = false;
                break;
                
            case 0x00:
                type += "*";	// Bit 7: Closed flag  (Not  set  produces  "*", or "splat" files)
                hide = true;
                break;					
        }
        
        uint8_t block_spc = 3;
        if (entry.blocks > 9) block_spc--;
        if (entry.blocks > 99) block_spc--;
        if (entry.blocks > 999) block_spc--;
        
        if ( !hide || show_hidden)
        {
            // line = sprintf("%s%-19s%s", str_repeat(" ", block_spc), "\"".$filename."\"", $type);
            // $this->sendLine( $blocks, $line, $type );
        }

        index++;
    } while ( entry.next_track != 0x00 && entry.next_sector != 0xFF );
    
    sendFooter();
}


Entry D64File::seekFile( std::string filename )
{
    Entry entry;

    // Read Directory Entries
    seekSector( directory_list_offset );
    do
    {		
        // Read Entry From Stream
        
        if (entry.file_type & 0b00000111 && filename == "*")
        {
            filename = entry.filename;
        }
        
        if ( filename == entry.filename )
        {
            // Move stream pointer to start track/sector
            return entry;
        }
    } while ( entry.file_type > 0 );
    
    entry.next_track = 0;
    entry.next_sector = 0;
    entry.blocks = 0;
    entry.filename = "";
    return entry;
}

void D64File::sendFile( std::string filename )
{
    Entry entry = seekFile( filename );
    seekSector( entry.start_track, entry.start_sector );

    bool last_block = false;
    do
    {
        uint8_t next_track = 0;  // Read first byte of sector
        uint8_t next_sector = 0; // Read second byte of sector
        
        if (next_track == 0) // Is this the last block?
        {
            //echo fread($this->fp, $next_sector); // Read number of bytes specified by next_sector since this is the last block
            last_block = true;
        }
        else
        {
            // echo fread($this->fp, 254);  // Read next 254 bytes
            seekSector( next_track, next_sector);
        }
    } while ( !last_block );
}


/********************************************************
 * File impls
 ********************************************************/

bool D64File::isDirectory() {
    // D64 don't support dirs!
    return false;
};

MIStream* D64File::createIStream(MIStream* is) {
    // has to return OPENED stream
    Debug_printv("[%s]", url.c_str());
    MIStream* istream = new D64IStream(url);
    istream->open();   
    return istream;
}

MOStream* D64File::outputStream() {
    // has to return OPENED stream
    MOStream* ostream = new D64OStream(url);
    ostream->open();   
    return ostream;
} ; 

time_t D64File::getLastWrite() {
    return 0;
} ;

time_t D64File::getCreationTime() {
    return 0;
} ;

bool D64File::exists() {
    // here I'd rather use D64 logic to see if such file name exists in the image!

    // Debug_printv("[%s]", url.c_str());
    // // we may try open the stream to check if it exists
    // std::unique_ptr<MIStream> test(inputStream());
    // // remember that MIStream destuctor should close the stream!
    // return test->isOpen();
} ; 

size_t D64File::size() {
    // use D64 to get size of the file in image
    // // we may take content-lenght from header if exists
    // std::unique_ptr<MIStream> test(inputStream());

    // size_t size = 0;

    // if(test->isOpen())
    //     size = test->available();

    // test->close();

    // return size;
};

/********************************************************
 * Istream impls
 ********************************************************/


bool D64IStream::seekPath(std::string path) {
    // Implement this to skip a queue of file streams to start of file by name
    // this will cause the next read to return bytes of 'path'
    return false;
};

std::string D64IStream::seekNextEntry() {
    // Implement this to skip a queue of file streams to start of next file and return its name
    // this will cause the next read to return bytes of "next" file in D64 image
    // might not have sense in this case, as D64 is kinda random access, not a stream.
    return "";
};

bool D64IStream::seek(uint32_t pos) {
    // seek only within current "active" ("seeked") file within the image (see above)
    if(pos==m_position)
        return true;

    return false;
};

size_t D64IStream::position() {
    return m_position; // return position within "seeked" file, not the D64 image!
};

void D64IStream::close() {
    ledOFF();
};

bool D64IStream::open() {
    // return true if we were able to read the image and confirmed it is valid.
    // it's up to you in what state the stream will be after open. Could be either:
    // 1. EOF-like state (0 available) and the state will be cleared only after succesful seekNextEntry or seekPath
    // 2. non-EOF-like state, and ready to send bytes of first file, because you did immediate seekNextEntry here
    
    return true;
};

int D64IStream::available() {
    // return bytes available in currently "seeked" file
    return m_bytesAvailable;
};

size_t D64IStream::size() {
    // size of the "seeked" file, not the image.
    return m_length;
};

size_t D64IStream::read(uint8_t* buf, size_t size) {
    // if we have the stream set to a specific file already, either via seekNextEntry or seekPath, return bytes of the file here
    // or set the stream to EOF-like state, if whle file is completely read.
    // as soon as we do seekNextEntry or seekPath, reading this stream will be possible again (available set to file size), as if nothing happened! 
    auto bytesRead= m_file.readBytes((char *) buf, size);
    m_bytesAvailable = m_file.available();
    m_position+=bytesRead;
    ledToggle(true);
    return bytesRead;
};

bool D64IStream::isOpen() {
    return m_isOpen;
};

