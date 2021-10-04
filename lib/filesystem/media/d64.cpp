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
    // hey, why not?
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
    Debug_printv("[%s]", url.c_str());
    // we may try open the stream to check if it exists
    std::unique_ptr<MIStream> test(inputStream());
    // remember that MIStream destuctor should close the stream!
    return test->isOpen();
} ; 

size_t D64File::size() {
    // we may take content-lenght from header if exists
    std::unique_ptr<MIStream> test(inputStream());

    size_t size = 0;

    if(test->isOpen())
        size = test->available();

    test->close();

    return size;
};

void D64File::fillPaths(std::vector<std::string>::iterator* matchedElement, std::vector<std::string>::iterator* fromStart, std::vector<std::string>::iterator* last) {
    //Serial.println("w fillpaths");   

    (*matchedElement)++;

    //Serial.println("w fillpaths stream pths");
    //streamPath = mstr::joinToString(fromStart, matchedElement, "/");
    streamPath = url;
    //Serial.println("w fillpaths path in stream");   
    //pathInStream = mstr::joinToString(matchedElement, last, "/");
    pathInStream = "";

    //Serial.printf("streamSrc='%s'\npathInStream='%s'\n", streamPath.c_str(), pathInStream.c_str());
}

// void D64File::addHeader(const String& name, const String& value, bool first, bool replace) {
//     //m_http.addHeader
// }


/********************************************************
 * Ostream impls
 ********************************************************/

size_t D64OStream::position() { return 0; };
void D64OStream::close() {
    m_http.end();
};
bool D64OStream::open() {
    // we'll ad a lambda that will allow adding headers
    // m_http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    mstr::replaceAll(url, "HTTP:", "http:");
    bool initOk = m_http.begin(m_file, url.c_str());
    Debug_printv("[%s] initOk[%d]", url.c_str(), initOk);
    if(!initOk)
        return false;
    
    //int httpCode = m_http.PUT(); //Send the request
//Serial.printf("URLSTR: httpCode=%d\n", httpCode);
    // if(httpCode != 200)
    //     return false;

    m_isOpen = true;
    //m_file = m_http.getStream();  //Get the response payload as Stream
    return true;
};
//size_t D64OStream::write(uint8_t) {};
size_t D64OStream::write(const uint8_t *buf, size_t size) {
    return m_file.write(buf, size);
};

bool D64OStream::isOpen() {
    return m_isOpen;
};


/********************************************************
 * Istream impls
 ********************************************************/

bool D64IStream::seek(uint32_t pos) {
    if(pos==m_position)
        return true;

    if(isFriendlySkipper) {
        char str[40];
        // Range: bytes=666-
        snprintf(str, sizeof str, "bytes=%lu-", (unsigned long)pos);
        m_http.addHeader("range",str);
        int httpCode = m_http.GET(); //Send the request
        Debug_printv("httpCode[%d]", httpCode);
        if(httpCode != 200)
            return false;

        Debug_printv("stream opened[%s]", url.c_str());
        m_file = m_http.getStream();  //Get the response payload as Stream
        m_position = pos;
        m_bytesAvailable = m_length-pos;
        return true;
    } else {
        if(pos<m_position) {
            // skipping backward and range not supported, let's simply reopen the stream...
            m_http.end();
            bool op = open();
            if(!op)
                return false;
        }

        m_position = 0;
        // ... and then read until we reach pos
        // while(m_position < pos) {
        //  m_position+=m_file.readBytes(buffer, size);  <----------- trurn this on!!!!
        // }
        m_bytesAvailable = m_length-pos;

        return true;
    }
};

size_t D64IStream::position() {
    return m_position;
};

void D64IStream::close() {
    ledOFF();
};

bool D64IStream::open() {

    m_isOpen = true;
    Debug_printv("[%s]", url.c_str());
    // m_length = m_http.getSize();
    Debug_printv("length=%d", m_length);
    m_bytesAvailable = m_length;

    return true;
};

int D64IStream::available() {
    return m_bytesAvailable;
};

size_t D64IStream::size() {
    return m_length;
};

size_t D64IStream::read(uint8_t* buf, size_t size) {
    auto bytesRead= m_file.readBytes((char *) buf, size);
    m_bytesAvailable = m_file.available();
    m_position+=bytesRead;
    ledToggle(true);
    return bytesRead;
};

bool D64IStream::isOpen() {
    return m_isOpen;
};
