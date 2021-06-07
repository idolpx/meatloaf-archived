#include "http.h"

/********************************************************
 * File impls
 ********************************************************/

bool HttpFile::isDirectory() {
    // hey, why not?
    //return m_path.endsWith("/");
    return m_path.back() == '/';
};

MIstream* HttpFile::inputStream() {
    // has to return OPENED stream
    MIstream* istream = new HttpIStream(m_path);
    istream->open();   
    return istream;
} ; 

MIstream* HttpFile::createIStream(MIstream* is) {
    return is; // we don't have to process this stream in any way, just return the original stream
}

MOstream* HttpFile::outputStream() {
    // has to return OPENED stream
    MOstream* ostream = new HttpOStream(m_path);
    ostream->open();   
    return ostream;

} ; 

time_t HttpFile::getLastWrite() {
    return 0;    
} ;

time_t HttpFile::getCreationTime() {
    return 0;
} ;

bool HttpFile::exists() {
    // we may try open the stream to check if it exists
    std::unique_ptr<MIstream> test(inputStream());

    return test->isOpen();
    // remember that MIStream destuctor should close the stream!
} ; 

size_t HttpFile::size() {
    // we may take content-lenght from header if exists
    return 0;
} ; 



/********************************************************
 * Ostream impls
 ********************************************************/

bool HttpOStream::seek(uint32_t pos, SeekMode mode) {};
bool HttpOStream::seek(uint32_t pos) {};
size_t HttpOStream::position() {};
void HttpOStream::close() {};
bool HttpOStream::open() {};
size_t HttpOStream::write(uint8_t) {};
size_t HttpOStream::write(const uint8_t *buf, size_t size) {};
void HttpOStream::flush() {};
bool HttpOStream::isOpen() {};


/********************************************************
 * Istream impls
 ********************************************************/

bool HttpIStream::seek(uint32_t pos, SeekMode mode) {};
bool HttpIStream::seek(uint32_t pos) {};
size_t HttpIStream::position() {};
void HttpIStream::close() {};
bool HttpIStream::open() {};
int HttpIStream::available() {};
uint8_t HttpIStream::read() {};
size_t HttpIStream::read(uint8_t* buf, size_t size) {};
bool HttpIStream::isOpen() {};
